/*
 * Copyright 2008 Arsen Chaloyan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <apr_general.h>
#include <sofia-sip/sdp.h>

#include "mrcp_unirtsp_server_agent.h"
#include "mrcp_session.h"
#include "mrcp_session_descriptor.h"
#include "mrcp_message.h"
#include "mrcp_resource_factory.h"
#include "rtsp_server.h"
#include "mrcp_unirtsp_sdp.h"
#include "apt_consumer_task.h"
#include "apt_log.h"

typedef struct mrcp_unirtsp_agent_t mrcp_unirtsp_agent_t;
typedef struct mrcp_unirtsp_session_t mrcp_unirtsp_session_t;

struct mrcp_unirtsp_agent_t {
	mrcp_sig_agent_t     *sig_agent;
	rtsp_server_t        *rtsp_server;

	rtsp_server_config_t *config;
};

struct mrcp_unirtsp_session_t {
	mrcp_session_t        *mrcp_session;
	rtsp_server_session_t *rtsp_session;
	su_home_t             *home;
};


static apt_bool_t server_destroy(apt_task_t *task);
static void server_on_start_complete(apt_task_t *task);
static void server_on_terminate_complete(apt_task_t *task);


static apt_bool_t mrcp_unirtsp_on_session_answer(mrcp_session_t *session, mrcp_session_descriptor_t *descriptor);
static apt_bool_t mrcp_unirtsp_on_session_terminate(mrcp_session_t *session);
static apt_bool_t mrcp_unirtsp_on_session_control(mrcp_session_t *session, mrcp_message_t *message);

static const mrcp_session_response_vtable_t session_response_vtable = {
	mrcp_unirtsp_on_session_answer,
	mrcp_unirtsp_on_session_terminate,
	mrcp_unirtsp_on_session_control
};

static apt_bool_t mrcp_unirtsp_session_create(rtsp_server_t *server, rtsp_server_session_t *session);
static apt_bool_t mrcp_unirtsp_session_terminate(rtsp_server_t *server, rtsp_server_session_t *session);
static apt_bool_t mrcp_unirtsp_message_handle(rtsp_server_t *server, rtsp_server_session_t *session, rtsp_message_t *message);

static const rtsp_server_vtable_t session_request_vtable = {
	mrcp_unirtsp_session_create,
	mrcp_unirtsp_session_terminate,
	mrcp_unirtsp_message_handle
};


static apt_bool_t rtsp_config_validate(mrcp_unirtsp_agent_t *agent, rtsp_server_config_t *config, apr_pool_t *pool);


/** Create UniRTSP Signaling Agent */
MRCP_DECLARE(mrcp_sig_agent_t*) mrcp_unirtsp_server_agent_create(rtsp_server_config_t *config, apr_pool_t *pool)
{
	apt_task_vtable_t vtable;
	apt_task_msg_pool_t *msg_pool;
	apt_consumer_task_t *consumer_task;
	mrcp_unirtsp_agent_t *agent;
	agent = apr_palloc(pool,sizeof(mrcp_unirtsp_agent_t));
	agent->sig_agent = mrcp_signaling_agent_create(agent,MRCP_VERSION_1,pool);
	agent->config = config;

	if(rtsp_config_validate(agent,config,pool) == FALSE) {
		return NULL;
	}

	agent->rtsp_server = rtsp_server_create(config->local_ip,config->local_port,config->max_connection_count,
										agent,&session_request_vtable,pool);
	if(!agent->rtsp_server) {
		return NULL;
	}

	msg_pool = apt_task_msg_pool_create_dynamic(0,pool);

	apt_task_vtable_reset(&vtable);
	vtable.destroy = server_destroy;
	vtable.on_start_complete = server_on_start_complete;
	vtable.on_terminate_complete = server_on_terminate_complete;
	consumer_task = apt_consumer_task_create(agent,&vtable,msg_pool,pool);
	agent->sig_agent->task = apt_consumer_task_base_get(consumer_task);
	apt_log(APT_PRIO_NOTICE,"Create UniRTSP Agent %s:%hu [%d]",
		config->local_ip,
		config->local_port,
		config->max_connection_count);
	return agent->sig_agent;
}

/** Allocate UniRTSP config */
MRCP_DECLARE(rtsp_server_config_t*) mrcp_unirtsp_server_config_alloc(apr_pool_t *pool)
{
	rtsp_server_config_t *config = apr_palloc(pool,sizeof(rtsp_server_config_t));
	config->local_ip = NULL;
	config->local_port = 0;
	config->origin = NULL;
	config->resource_location = NULL;
	config->max_connection_count = 100;
	return config;
}


static apt_bool_t rtsp_config_validate(mrcp_unirtsp_agent_t *agent, rtsp_server_config_t *config, apr_pool_t *pool)
{
	agent->config = config;
	return TRUE;
}

static APR_INLINE mrcp_unirtsp_agent_t* server_agent_get(apt_task_t *task)
{
	apt_consumer_task_t *consumer_task = apt_task_object_get(task);
	mrcp_unirtsp_agent_t *agent = apt_consumer_task_object_get(consumer_task);
	return agent;
}

static apt_bool_t server_destroy(apt_task_t *task)
{
	mrcp_unirtsp_agent_t *agent = server_agent_get(task);
	if(agent->rtsp_server) {
		rtsp_server_destroy(agent->rtsp_server);
		agent->rtsp_server = NULL;
	}
	return TRUE;
}

static void server_on_start_complete(apt_task_t *task)
{
	mrcp_unirtsp_agent_t *agent = server_agent_get(task);
	if(agent->rtsp_server) {
		rtsp_server_start(agent->rtsp_server);
	}
}

static void server_on_terminate_complete(apt_task_t *task)
{
	mrcp_unirtsp_agent_t *agent = server_agent_get(task);
	if(agent->rtsp_server) {
		rtsp_server_terminate(agent->rtsp_server);
	}
}

static apt_bool_t mrcp_unirtsp_session_create(rtsp_server_t *rtsp_server, rtsp_server_session_t *rtsp_session)
{
	mrcp_unirtsp_agent_t *agent = rtsp_server_object_get(rtsp_server);
	const apt_str_t *session_id;
	mrcp_unirtsp_session_t *session;
	mrcp_session_t* mrcp_session = agent->sig_agent->create_server_session(agent->sig_agent);
	if(!mrcp_session) {
		return FALSE;
	}
	session_id = rtsp_server_session_id_get(rtsp_session);
	if(session_id) {
		mrcp_session->id = *session_id;
	}
	mrcp_session->response_vtable = &session_response_vtable;
	mrcp_session->event_vtable = NULL;

	session = apr_palloc(mrcp_session->pool,sizeof(mrcp_unirtsp_session_t));
	session->mrcp_session = mrcp_session;
	mrcp_session->obj = session;
	
	session->home = su_home_new(sizeof(*session->home));

	rtsp_server_session_object_set(rtsp_session,session);
	session->rtsp_session = rtsp_session;
	return TRUE;
}

static apt_bool_t mrcp_unirtsp_session_terminate(rtsp_server_t *rtsp_server, rtsp_server_session_t *rtsp_session)
{
	mrcp_unirtsp_session_t *session	= rtsp_server_session_object_get(rtsp_session);
	if(!session) {
		return FALSE;
	}
	return mrcp_session_terminate_request(session->mrcp_session);
}

static void mrcp_unirtsp_session_destroy(mrcp_unirtsp_session_t *session)
{
	if(session->home) {
		su_home_unref(session->home);
		session->home = NULL;
	}
	rtsp_server_session_object_set(session->rtsp_session,NULL);
	mrcp_session_destroy(session->mrcp_session);
}

static apt_bool_t mrcp_unirtsp_session_announce(mrcp_unirtsp_agent_t *agent, mrcp_unirtsp_session_t *session, rtsp_message_t *message)
{
	const char *resource_name = message->start_line.common.request_line.resource_name;
	apt_bool_t status = TRUE;

	if(session && resource_name &&
		rtsp_header_property_check(&message->header.property_set,RTSP_HEADER_FIELD_CONTENT_TYPE) == TRUE &&
		message->header.content_type == RTSP_CONTENT_TYPE_MRCP &&
		rtsp_header_property_check(&message->header.property_set,RTSP_HEADER_FIELD_CONTENT_LENGTH) == TRUE &&
		message->header.content_length > 0) {

		apt_text_stream_t text_stream;
		mrcp_message_t *mrcp_message;

		text_stream.text = message->body;
		text_stream.pos = text_stream.text.buf;

		mrcp_message = mrcp_message_create(session->mrcp_session->pool);
		mrcp_message->channel_id.session_id = message->header.session_id;
		apt_string_assign(&mrcp_message->channel_id.resource_name,resource_name,mrcp_message->pool);
		if(mrcp_message_parse(agent->sig_agent->resource_factory,mrcp_message,&text_stream) == TRUE) {
			status = mrcp_session_control_request(session->mrcp_session,mrcp_message);
		}
		else {
			/* error response */
			apt_log(APT_PRIO_WARNING,"Failed to Parse MRCPv1 Message");
			status = FALSE;
		}
	}
	else {
		/* error response */
		status = FALSE;
	}
	return status;
}

static apt_bool_t mrcp_unirtsp_message_handle(rtsp_server_t *rtsp_server, rtsp_server_session_t *rtsp_session, rtsp_message_t *rtsp_message)
{
	apt_bool_t status = FALSE;
	mrcp_unirtsp_session_t *session	= rtsp_server_session_object_get(rtsp_session);
	if(!session) {
		return FALSE;
	}

	switch(rtsp_message->start_line.common.request_line.method_id) {
		case RTSP_METHOD_SETUP:
		case RTSP_METHOD_TEARDOWN:
		{
			mrcp_session_descriptor_t *descriptor;
			descriptor = mrcp_descriptor_generate_by_rtsp_request(rtsp_message,session->mrcp_session->pool,session->home);
			status = mrcp_session_offer(session->mrcp_session,descriptor);
			break;
		}
		case RTSP_METHOD_ANNOUNCE:
		{
			mrcp_unirtsp_agent_t *agent = rtsp_server_object_get(rtsp_server);
			status = mrcp_unirtsp_session_announce(agent,session,rtsp_message);
			break;
		}
		default:
			break;
	}

	return status;
}

static apt_bool_t mrcp_unirtsp_on_session_answer(mrcp_session_t *mrcp_session, mrcp_session_descriptor_t *descriptor)
{
	mrcp_unirtsp_session_t *session = mrcp_session->obj;
	mrcp_unirtsp_agent_t *agent = mrcp_session->signaling_agent->obj;
	rtsp_message_t *response = NULL;
	const rtsp_message_t *request = rtsp_server_session_request_get(session->rtsp_session);
	if(!request) {
		return FALSE;
	}

	if(agent->config->origin) {
		apt_string_set(&descriptor->origin,agent->config->origin);
	}

	if(request->start_line.common.request_line.method_id == RTSP_METHOD_SETUP) {
		response = rtsp_response_generate_by_mrcp_descriptor(request,descriptor,mrcp_session->pool);
	}
	else if(request->start_line.common.request_line.method_id == RTSP_METHOD_TEARDOWN) {
		response = rtsp_response_create(request,RTSP_STATUS_CODE_OK,RTSP_REASON_PHRASE_OK,mrcp_session->pool);
	}

	if(!response) {
		return FALSE;
	}
	rtsp_server_session_respond(agent->rtsp_server,session->rtsp_session,response);
	return TRUE;
}

static apt_bool_t mrcp_unirtsp_on_session_terminate(mrcp_session_t *mrcp_session)
{
	mrcp_unirtsp_session_t *session = mrcp_session->obj;
	mrcp_unirtsp_agent_t *agent = mrcp_session->signaling_agent->obj;

	rtsp_server_session_terminate(agent->rtsp_server,session->rtsp_session);
	mrcp_unirtsp_session_destroy(session);
	return TRUE;
}

static apt_bool_t mrcp_unirtsp_on_session_control(mrcp_session_t *mrcp_session, mrcp_message_t *mrcp_message)
{
	mrcp_unirtsp_session_t *session = mrcp_session->obj;
	mrcp_unirtsp_agent_t *agent = mrcp_session->signaling_agent->obj;
	char buffer[4096];
	apt_text_stream_t text_stream;
	rtsp_message_t *rtsp_message = NULL;

	text_stream.text.buf = buffer;
	text_stream.text.length = sizeof(buffer)-1;
	text_stream.pos = text_stream.text.buf;

	mrcp_message->start_line.version = MRCP_VERSION_1;
	if(mrcp_message_generate(agent->sig_agent->resource_factory,mrcp_message,&text_stream) != TRUE) {
		apt_log(APT_PRIO_WARNING,"Failed to Generate MRCPv1 Message");
		return FALSE;
	}
	*text_stream.pos = '\0';

	if(mrcp_message->start_line.message_type == MRCP_MESSAGE_TYPE_RESPONSE) {
		/* send RTSP response (OK) */
		const rtsp_message_t *request = rtsp_server_session_request_get(session->rtsp_session);
		if(request) {
			rtsp_message = rtsp_response_create(request,RTSP_STATUS_CODE_OK,RTSP_REASON_PHRASE_OK,mrcp_session->pool);
		}
	}
	else if(mrcp_message->start_line.message_type == MRCP_MESSAGE_TYPE_EVENT) {
		/* send RTSP announce */
		rtsp_message = rtsp_request_create(mrcp_session->pool);
		rtsp_message->start_line.common.request_line.method_id = RTSP_METHOD_ANNOUNCE;
	}

	if(!rtsp_message) {
		return FALSE;
	}

	apt_string_copy(&rtsp_message->body,&text_stream.text,rtsp_message->pool);
	rtsp_message->header.content_type = RTSP_CONTENT_TYPE_MRCP;
	rtsp_header_property_add(&rtsp_message->header.property_set,RTSP_HEADER_FIELD_CONTENT_TYPE);
	rtsp_message->header.content_length = text_stream.text.length;
	rtsp_header_property_add(&rtsp_message->header.property_set,RTSP_HEADER_FIELD_CONTENT_LENGTH);

	rtsp_server_session_respond(agent->rtsp_server,session->rtsp_session,rtsp_message);
	return TRUE;
}
