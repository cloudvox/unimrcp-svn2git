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

#include <stdio.h>
#include "mrcp_header_accessor.h"


MRCP_DECLARE(apt_bool_t) mrcp_header_field_parse(mrcp_header_accessor_t *accessor, const apt_header_field_t *header_field, apr_size_t *id, apr_pool_t *pool)
{
	if(!accessor->vtable || !id) {
		return FALSE;
	}

	*id = apt_string_table_id_find(accessor->vtable->field_table,accessor->vtable->field_count,&header_field->name);
	if(*id >= accessor->vtable->field_count) {
		return FALSE;
	}

	if(header_field->value.length) {
		if(accessor->vtable->parse_field(accessor,*id,&header_field->value,pool) == FALSE) {
			return FALSE;
		}
	}
	
	return TRUE;
}

MRCP_DECLARE(apt_header_field_t*) mrcp_header_field_generate(mrcp_header_accessor_t *accessor, apr_size_t id, apt_bool_t empty_value, apr_pool_t *pool)
{
	apt_header_field_t *header_field;
	const apt_str_t *name;

	if(!accessor->vtable) {
		return NULL;
	}
	
	header_field = apr_palloc(pool,sizeof(apt_header_field_t));
	name = apt_string_table_str_get(accessor->vtable->field_table,accessor->vtable->field_count,id);
	if(name) {
		header_field->name = *name;
	}
	else {
		apt_string_reset(&header_field->name);
	}

	if(empty_value == FALSE) {
		char buffer[256];
		apt_str_t *value;
		apt_text_stream_t stream;
		apt_text_stream_init(&stream,buffer,sizeof(buffer));
		if(accessor->vtable->generate_field(accessor,id,&stream) == FALSE) {
			return NULL;
		}
	
		value = &header_field->value;
		value->length = stream.pos - stream.text.buf;
		value->buf = apr_palloc(pool,value->length + 1);
		memcpy(value->buf,stream.text.buf,value->length);
		value->buf[value->length] = '\0';
	}
	else {
		apt_string_reset(&header_field->value);
	}

	return header_field;
}


/** Generate completion-cause */
MRCP_DECLARE(apt_bool_t) mrcp_completion_cause_generate(const apt_str_table_item_t table[], apr_size_t size, apr_size_t cause, apt_text_stream_t *stream)
{
	int length;
	const apt_str_t *name = apt_string_table_str_get(table,size,cause);
	if(!name) {
		return FALSE;
	}
	length = sprintf(stream->pos,"%03"APR_SIZE_T_FMT" ",cause);
	if(length <= 0) {
		return FALSE;
	}
	stream->pos += length;

	memcpy(stream->pos,name->buf,name->length);
	stream->pos += name->length;
	return TRUE;
}
