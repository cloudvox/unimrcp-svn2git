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

#ifndef MRCP_MESSAGE_H
#define MRCP_MESSAGE_H

/**
 * @file mrcp_message.h
 * @brief MRCP Message Definition
 */ 

#include "mrcp_types.h"
#include "mrcp_start_line.h"
#include "mrcp_header_accessor.h"
#include "mrcp_generic_header.h"

APT_BEGIN_EXTERN_C

/** MRCP channel-id declaration */
typedef struct mrcp_channel_id mrcp_channel_id;
/** MRCP message header declaration */
typedef struct mrcp_message_header_t mrcp_message_header_t;

/** MRCP channel-identifier */
struct mrcp_channel_id {
	/** Unambiguous string identifying the MRCP session */
	apt_str_t        session_id;
	/** MRCP resource name */
	apt_str_t        resource_name;
};

/** Initialize MRCP channel-identifier */
MRCP_DECLARE(void) mrcp_channel_id_init(mrcp_channel_id *channel_id);

/** Parse MRCP channel-identifier */
MRCP_DECLARE(apt_bool_t) mrcp_channel_id_parse(mrcp_channel_id *channel_id, apt_text_stream_t *text_stream, apr_pool_t *pool);

/** Generate MRCP channel-identifier */
MRCP_DECLARE(apt_bool_t) mrcp_channel_id_generate(mrcp_channel_id *channel_id, apt_text_stream_t *text_stream);


/** MRCP message-header */
struct mrcp_message_header_t {
	/** MRCP generic-header */
	mrcp_header_accessor_t generic_header_accessor;
	/** MRCP resource specific header */
	mrcp_header_accessor_t resource_header_accessor;

	/** Header section (collection of header fields)*/
	apt_header_section_t   header_section;
};

/** Initialize MRCP message-header */
static APR_INLINE void mrcp_message_header_init(mrcp_message_header_t *message_header)
{
	mrcp_header_accessor_init(&message_header->generic_header_accessor);
	mrcp_header_accessor_init(&message_header->resource_header_accessor);
}

/** Destroy MRCP message-header */
static APR_INLINE void mrcp_message_header_destroy(mrcp_message_header_t *message_header)
{
	mrcp_header_destroy(&message_header->generic_header_accessor);
	mrcp_header_destroy(&message_header->resource_header_accessor);
}


/** Parse MRCP message-header */
MRCP_DECLARE(apt_bool_t) mrcp_message_header_parse(mrcp_message_header_t *message_header, apt_text_stream_t *text_stream, apr_pool_t *pool);

/** Generate MRCP message-header */
MRCP_DECLARE(apt_bool_t) mrcp_message_header_generate(mrcp_message_header_t *message_header, apt_text_stream_t *text_stream);

/** Set MRCP message-header */
MRCP_DECLARE(apt_bool_t) mrcp_message_header_set(mrcp_message_header_t *message_header, const mrcp_message_header_t *src, apr_pool_t *pool);

/** Get MRCP message-header */
MRCP_DECLARE(apt_bool_t) mrcp_message_header_get(mrcp_message_header_t *message_header, const mrcp_message_header_t *src, apr_pool_t *pool);

/** Inherit MRCP message-header */
MRCP_DECLARE(apt_bool_t) mrcp_message_header_inherit(mrcp_message_header_t *message_header, const mrcp_message_header_t *parent, apr_pool_t *pool);



/** MRCP message */
struct mrcp_message_t {
	/** Start-line of MRCP message */
	mrcp_start_line_t     start_line;
	/** Channel-identifier of MRCP message */
	mrcp_channel_id       channel_id;
	/** Header of MRCP message */
	mrcp_message_header_t header;
	/** Body of MRCP message */
	apt_str_t             body;

	/** Associated MRCP resource */
	mrcp_resource_t      *resource;
	/** Memory pool MRCP message is allocated from */
	apr_pool_t           *pool;
};

/** Create MRCP message */
MRCP_DECLARE(mrcp_message_t*) mrcp_message_create(apr_pool_t *pool);

/** Create MRCP request message */
MRCP_DECLARE(mrcp_message_t*) mrcp_request_create(mrcp_resource_t *resource, mrcp_version_e version, mrcp_method_id method_id, apr_pool_t *pool);
/** Create MRCP response message */
MRCP_DECLARE(mrcp_message_t*) mrcp_response_create(const mrcp_message_t *request_message, apr_pool_t *pool);
/** Create MRCP event message */
MRCP_DECLARE(mrcp_message_t*) mrcp_event_create(const mrcp_message_t *request_message, mrcp_method_id event_id, apr_pool_t *pool);

/** Associate MRCP resource specific data by resource name */
MRCP_DECLARE(apt_bool_t) mrcp_message_resource_set(mrcp_message_t *message, mrcp_resource_t *resource);

/** Validate MRCP message */
MRCP_DECLARE(apt_bool_t) mrcp_message_validate(mrcp_message_t *message);

/** Destroy MRCP message */
MRCP_DECLARE(void) mrcp_message_destroy(mrcp_message_t *message);


/** Parse MRCP message-body */
MRCP_DECLARE(apt_bool_t) mrcp_body_parse(mrcp_message_t *message, apt_text_stream_t *text_stream, apr_pool_t *pool);
/** Generate MRCP message-body */
MRCP_DECLARE(apt_bool_t) mrcp_body_generate(mrcp_message_t *message, apt_text_stream_t *text_stream);

/** Get MRCP generic-header */
static APR_INLINE void* mrcp_generic_header_get(mrcp_message_t *mrcp_message)
{
	return mrcp_message->header.generic_header_accessor.data;
}

/** Prepare MRCP generic-header */
static APR_INLINE void* mrcp_generic_header_prepare(mrcp_message_t *mrcp_message)
{
	return mrcp_header_allocate(&mrcp_message->header.generic_header_accessor,mrcp_message->pool);
}

/** Add MRCP generic-header proprerty */
static APR_INLINE void mrcp_generic_header_property_add(mrcp_message_t *mrcp_message, apr_size_t id)
{
	apt_header_field_t *header_field = mrcp_header_field_generate(
										&mrcp_message->header.generic_header_accessor,
										id,
										FALSE,
										mrcp_message->pool);
	if(header_field) {
		apt_header_section_field_add(&mrcp_message->header.header_section,header_field,id);
	}
}

/** Add MRCP generic-header name only proprerty (should be used to construct empty headers in case of GET-PARAMS request) */
static APR_INLINE void mrcp_generic_header_name_property_add(mrcp_message_t *mrcp_message, apr_size_t id)
{
	apt_header_field_t *header_field = mrcp_header_field_generate(
										&mrcp_message->header.generic_header_accessor,
										id,
										TRUE,
										mrcp_message->pool);
	if(header_field) {
		apt_header_section_field_add(&mrcp_message->header.header_section,header_field,id);
	}
}

/** Check MRCP generic-header proprerty */
static APR_INLINE apt_bool_t mrcp_generic_header_property_check(mrcp_message_t *mrcp_message, apr_size_t id)
{
	return apt_header_section_field_check(&mrcp_message->header.header_section,id);
}


/** Get MRCP resource-header */
static APR_INLINE void* mrcp_resource_header_get(const mrcp_message_t *mrcp_message)
{
	return mrcp_message->header.resource_header_accessor.data;
}

/** Prepare MRCP resource-header */
static APR_INLINE void* mrcp_resource_header_prepare(mrcp_message_t *mrcp_message)
{
	return mrcp_header_allocate(&mrcp_message->header.resource_header_accessor,mrcp_message->pool);
}

/** Add MRCP resource-header proprerty */
static APR_INLINE void mrcp_resource_header_property_add(mrcp_message_t *mrcp_message, apr_size_t id)
{
	apt_header_field_t *header_field = mrcp_header_field_generate(
										&mrcp_message->header.resource_header_accessor,
										id,
										FALSE,
										mrcp_message->pool);
	if(header_field) {
		apt_header_section_field_add(&mrcp_message->header.header_section,header_field,id + GENERIC_HEADER_COUNT);
	}
}

/** Add MRCP resource-header name only proprerty (should be used to construct empty headers in case of GET-PARAMS request) */
static APR_INLINE void mrcp_resource_header_name_property_add(mrcp_message_t *mrcp_message, apr_size_t id)
{
	apt_header_field_t *header_field = mrcp_header_field_generate(
										&mrcp_message->header.resource_header_accessor,
										id,
										TRUE,
										mrcp_message->pool);
	if(header_field) {
		apt_header_section_field_add(&mrcp_message->header.header_section,header_field,id + GENERIC_HEADER_COUNT);
	}
}

/** Check MRCP resource-header proprerty */
static APR_INLINE apt_bool_t mrcp_resource_header_property_check(mrcp_message_t *mrcp_message, apr_size_t id)
{
	return apt_header_section_field_check(&mrcp_message->header.header_section,id + GENERIC_HEADER_COUNT);
}



APT_END_EXTERN_C

#endif /* MRCP_MESSAGE_H */
