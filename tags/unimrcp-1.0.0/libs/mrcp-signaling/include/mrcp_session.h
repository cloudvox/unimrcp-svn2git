/*
 * Copyright 2008-2010 Arsen Chaloyan
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
 * 
 * $Id$
 */

#ifndef MRCP_SESSION_H
#define MRCP_SESSION_H

/**
 * @file mrcp_session.h
 * @brief Abstract MRCP Session
 */ 

#include "mrcp_sig_types.h"
#include "apt_string.h"

APT_BEGIN_EXTERN_C

/** Macro to log session pointers */
#define MRCP_SESSION_PTR(session) (session)
/** Macro to log session string identifiers */
#define MRCP_SESSION_SID(session) \
	(session)->id.buf ? (session)->id.buf : "new"

/** Macro to log session pointers and string identifiers */
#define MRCP_SESSION_PTRSID(session) \
	MRCP_SESSION_PTR(session), MRCP_SESSION_SID(session)

/** MRCP session request vtable declaration */
typedef struct mrcp_session_request_vtable_t mrcp_session_request_vtable_t;
/** MRCP session response vtable declaration */
typedef struct mrcp_session_response_vtable_t mrcp_session_response_vtable_t;
/** MRCP session event vtable declaration */
typedef struct mrcp_session_event_vtable_t mrcp_session_event_vtable_t;


/** MRCP session */
struct mrcp_session_t {
	/** Memory pool to allocate memory from */
	apr_pool_t       *pool;
	/** External object associated with session */
	void             *obj;
	/** Informative name of the session used for debugging */
	const char       *name;
	/** Back pointer to signaling agent */
	mrcp_sig_agent_t *signaling_agent;

	/** Session identifier */
	apt_str_t         id;
	/** Last request identifier sent for client, received for server */
	mrcp_request_id   last_request_id;

	/** Virtual request methods */
	const mrcp_session_request_vtable_t  *request_vtable;
	/** Virtual response methods */
	const mrcp_session_response_vtable_t *response_vtable;
	/** Virtual event methods */
	const mrcp_session_event_vtable_t    *event_vtable;
};


/** MRCP session request vtable */
struct mrcp_session_request_vtable_t {
	/** Offer session descriptor */
	apt_bool_t (*offer)(mrcp_session_t *session, mrcp_session_descriptor_t *descriptor);
	/** Terminate session */
	apt_bool_t (*terminate)(mrcp_session_t *session);
	/** Control session (MRCPv1 only) */
	apt_bool_t (*control)(mrcp_session_t *session, mrcp_message_t *message);
	/** Discover resources */
	apt_bool_t (*discover)(mrcp_session_t *session, mrcp_session_descriptor_t *descriptor);
};

/** MRCP session response vtable */
struct mrcp_session_response_vtable_t {
	/** Answer with remote session descriptor */
	apt_bool_t (*on_answer)(mrcp_session_t *session, mrcp_session_descriptor_t *descriptor);
	/** Session terminated */
	apt_bool_t (*on_terminate)(mrcp_session_t *session);
	/** Control session (MRCPv1 only) */
	apt_bool_t (*on_control)(mrcp_session_t *session, mrcp_message_t *message);
	/** Response to resource discovery request */
	apt_bool_t (*on_discover)(mrcp_session_t *session, mrcp_session_descriptor_t *descriptor);
};

/** MRCP session event vtable */
struct mrcp_session_event_vtable_t {
	/** Received session termination event without appropriate request */
	apt_bool_t (*on_terminate)(mrcp_session_t *session);
};


/** Create new memory pool and allocate session object from the pool. */
MRCP_DECLARE(mrcp_session_t*) mrcp_session_create(apr_size_t padding);

/** Destroy session and assosiated memory pool. */
MRCP_DECLARE(void) mrcp_session_destroy(mrcp_session_t *session);


/** Offer */
static APR_INLINE apt_bool_t mrcp_session_offer(mrcp_session_t *session, mrcp_session_descriptor_t *descriptor)
{
	if(session->request_vtable->offer) {
		return session->request_vtable->offer(session,descriptor);
	}
	return FALSE;
}

/** Terminate */
static APR_INLINE apt_bool_t mrcp_session_terminate_request(mrcp_session_t *session)
{
	if(session->request_vtable->terminate) {
		return session->request_vtable->terminate(session);
	}
	return FALSE;
}

/** Answer */
static APR_INLINE apt_bool_t mrcp_session_answer(mrcp_session_t *session, mrcp_session_descriptor_t *descriptor)
{
	if(session->response_vtable->on_answer) {
		return session->response_vtable->on_answer(session,descriptor);
	}
	return FALSE;
}

/** On terminate response */
static APR_INLINE apt_bool_t mrcp_session_terminate_response(mrcp_session_t *session)
{
	if(session->response_vtable->on_terminate) {
		return session->response_vtable->on_terminate(session);
	}
	return FALSE;
}

/** On terminate event */
static APR_INLINE apt_bool_t mrcp_session_terminate_event(mrcp_session_t *session)
{
	if(session->event_vtable->on_terminate) {
		return session->event_vtable->on_terminate(session);
	}
	return FALSE;
}

/** Control request */
static APR_INLINE apt_bool_t mrcp_session_control_request(mrcp_session_t *session, mrcp_message_t *message)
{
	if(session->request_vtable->control) {
		return session->request_vtable->control(session,message);
	}
	return FALSE;
}

/** On control response/event */
static APR_INLINE apt_bool_t mrcp_session_control_response(mrcp_session_t *session, mrcp_message_t *message)
{
	if(session->response_vtable->on_control) {
		return session->response_vtable->on_control(session,message);
	}
	return FALSE;
}

/** Resource discovery request */
static APR_INLINE apt_bool_t mrcp_session_discover_request(mrcp_session_t *session, mrcp_session_descriptor_t *descriptor)
{
	if(session->request_vtable->discover) {
		return session->request_vtable->discover(session,descriptor);
	}
	return FALSE;
}

/** On resource discovery response */
static APR_INLINE apt_bool_t mrcp_session_discover_response(mrcp_session_t *session, mrcp_session_descriptor_t *descriptor)
{
	if(session->response_vtable->on_discover) {
		return session->response_vtable->on_discover(session,descriptor);
	}
	return FALSE;
}

APT_END_EXTERN_C

#endif /* MRCP_SESSION_H */
