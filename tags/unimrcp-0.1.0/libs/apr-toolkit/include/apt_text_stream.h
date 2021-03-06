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

#ifndef __APT_TEXT_STREAM_H__
#define __APT_TEXT_STREAM_H__

/**
 * @file apt_text_stream.h
 * @brief Text Stream Parse/Generate Routine
 */ 

#include <stdlib.h>
#include <stdio.h>
#include "apt_string.h"

APT_BEGIN_EXTERN_C

/** Named tokens */

/** Space */
#define APT_TOKEN_SP ' '
/** Carrige return */
#define APT_TOKEN_CR 0x0D
/** Line feed */
#define APT_TOKEN_LF 0x0A

/** Text stream declaration */
typedef struct apt_text_stream_t apt_text_stream_t;
/** Name-value declaration */
typedef struct apt_name_value_t apt_name_value_t;

/** Text stream is used for message parsing and generation */
struct apt_text_stream_t {
	/** Text stream */
	apt_str_t text;
	/** Current position in the buffer */
	char     *pos;
};

/** Generic name-value pair definition ("name:value") */
struct apt_name_value_t {
	/** The name */
	apt_str_t name;
	/** The value */
	apt_str_t value;
};

/**
 * Navigate through the lines of the text stream (message).
 * @param stream the text stream to navigate
 * @param line the read line to return
 * @return TRUE if the length of the line > 0, otherwise FALSE
 */
APT_DECLARE(apt_bool_t) apt_text_line_read(apt_text_stream_t *stream, apt_str_t *line);

/**
 * Navigate through the headers (name:value pairs) of the text stream (message).
 * @param stream the text stream to navigate
 * @param pair the read pair to return
 * @return TRUE if the length of the read name > 0, otherwise FALSE
 */
APT_DECLARE(apt_bool_t) apt_text_header_read(apt_text_stream_t *stream, apt_name_value_t *pair);

/**
 * Navigate through the fields of the line.
 * @param stream the text stream to navigate
 * @param separator the field separator
 * @param skip_spaces whether to skip spaces or not
 * @param field the read field to return
 * @return TRUE if the length of the field > 0, otherwise FALSE
 */
APT_DECLARE(apt_bool_t) apt_text_field_read(apt_text_stream_t *stream, char separator, apt_bool_t skip_spaces, apt_str_t *field);



/** Generate header */
APT_DECLARE(apt_bool_t) apt_text_header_generate(const apt_name_value_t *pair, apt_text_stream_t *text_stream);

/** Generate only the name ("name:") of the header */
APT_DECLARE(apt_bool_t) apt_text_header_name_generate(const apt_str_t *name, apt_text_stream_t *text_stream);



/** Parse boolean-value */
APT_DECLARE(apt_bool_t) apt_boolean_value_parse(const apt_str_t *str, apt_bool_t *value);

/** Generate boolean-value */
APT_DECLARE(apt_bool_t) apt_boolean_value_generate(apt_bool_t value, apt_text_stream_t *str);


/** Parse size_t value */
static APR_INLINE apr_size_t apt_size_value_parse(const apt_str_t *str)
{
	return str->buf ? atol(str->buf) : 0;
}

/** Generate apr_size_t value */
static APR_INLINE apt_bool_t apt_size_value_generate(apr_size_t value, apt_text_stream_t *stream)
{
	int length = sprintf(stream->pos, "%"APR_SIZE_T_FMT, value);
	if(length <= 0) {
		return FALSE;
	}
	stream->pos += length;
	return TRUE;
}

/** Parse float value */
static APR_INLINE float apt_float_value_parse(const apt_str_t *str)
{
	return str->buf ? (float)atof(str->buf) : 0;
}

/** Generate float value */
static APR_INLINE apt_bool_t apt_float_value_generate(float value, apt_text_stream_t *stream)
{
	int length = sprintf(stream->pos,"%.1f",value);
	if(length <= 0) {
		return FALSE;
	}
	stream->pos += length;
	return TRUE;
}

/** Generate string value */
static APR_INLINE apt_bool_t apt_string_value_generate(const apt_str_t *str, apt_text_stream_t *stream)
{
	if(str->length) {
		memcpy(stream->pos,str->buf,str->length);
		stream->pos += str->length;
	}
	return TRUE;
}


/** Insert end of the line symbol(s) */
static APR_INLINE void apt_text_eol_insert(apt_text_stream_t *stream)
{
	*stream->pos++ = APT_TOKEN_CR;
	*stream->pos++ = APT_TOKEN_LF;
}

/** Insert space */
static APR_INLINE void apt_text_space_insert(apt_text_stream_t *stream)
{
	*stream->pos++ = APT_TOKEN_SP;
}

/** Skip spaces */
static APR_INLINE void apt_text_spaces_skip(apt_text_stream_t *stream)
{
	const char *end = stream->text.buf + stream->text.length;
	while(stream->pos < end && *stream->pos == APT_TOKEN_SP) stream->pos++;
}

/** Parse id at resource string */
APT_DECLARE(apt_bool_t) apt_id_resource_parse(const apt_str_t *str, char separator, apt_str_t *id, apt_str_t *resource, apr_pool_t *pool);

/** Generate id at resource string */
APT_DECLARE(apt_bool_t) apt_id_resource_generate(const apt_str_t *id, const apt_str_t *resource, char separator, apt_str_t *str, apr_pool_t *pool);

/** Generate value plus the length (number of digits) of the value itself */
APT_DECLARE(apt_bool_t) apt_var_length_value_generate(apr_size_t *value, apr_size_t max_count, apt_str_t *str);


/**
 * Generate unique identifier (hex string)
 * @param id the id to generate
 * @param length the length of hex string to generate
 * @param pool the pool to allocate memory from
 */
APT_DECLARE(apt_bool_t) apt_unique_id_generate(apt_str_t *id, apr_size_t length, apr_pool_t *pool);


APT_END_EXTERN_C

#endif /*__APT_TEXT_STREAM_H__*/
