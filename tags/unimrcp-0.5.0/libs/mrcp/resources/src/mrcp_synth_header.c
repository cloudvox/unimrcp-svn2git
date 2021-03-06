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

#include "mrcp_synth_header.h"

/** String table of MRCP synthesizer headers (mrcp_synthesizer_header_id) */
static const apt_str_table_item_t synth_header_string_table[] = {
	{{"Jump-Size",            9},0},
	{{"Kill-On-Barge-In",    16},0},
	{{"Speaker-Profile",     15},6},
	{{"Completion-Cause",    16},16},
	{{"Completion-Reason",   17},11},
	{{"Voice-Gender",        12},6},
	{{"Voice-Age",            9},6},
	{{"Voice-Variant",       13},6},
	{{"Voice-Name",          10},6},
	{{"Prosody-Volume",      14},8},
	{{"Prosody-Rate",        12},8},
	{{"Speech-Marker",       13},7},
	{{"Speech-Language",     15},7},
	{{"Fetch-Hint",          10},6},
	{{"Fetch-Timeout",       13},6},
	{{"Audio-Fetch-Hint",    16},0},
	{{"Failed-Uri",          10},10},
	{{"Failed-Uri_Cause",    16},10},
	{{"Speak-Restart",       13},6},
	{{"Speak-Length",        12},6},
	{{"Load-Lexicon",        12},2},
	{{"Lexicon-Search-Order",20},2}
};

/** String table of MRCP speech-unit fields (mrcp_speech_unit_t) */
static const apt_str_table_item_t speech_unit_string_table[] = {
	{{"Second",   6},2},
	{{"Word",     4},0},
	{{"Sentence", 8},2},
	{{"Paragraph",9},0}
};

/** String table of MRCP voice-gender fields (mrcp_voice_gender_t) */
static const apt_str_table_item_t voice_gender_string_table[] = {
	{{"male",   4},0},
	{{"female", 6},0},
	{{"neutral",7},0}
};

/** String table of MRCP prosody-volume fields (mrcp_prosody_volume_t) */
static const apt_str_table_item_t prosody_volume_string_table[] = {
	{{"silent", 6},1},
	{{"x-soft", 6},2},
	{{"soft",   4},3},
	{{"medium", 6},0},
	{{"loud",   4},0},
	{{"x-loud", 6},5},
	{{"default",7},0} 
};

/** String table of MRCP prosody-rate fields (mrcp_prosody_rate_t) */
static const apt_str_table_item_t prosody_rate_string_table[] = {
	{{"x-slow", 6},3},
	{{"slow",   4},0},
	{{"medium", 6},0},
	{{"fast",   4},0},
	{{"x-fast", 6},4},
	{{"default",7},0}
};

/** String table of MRCP synthesizer completion-cause fields (mrcp_synthesizer_completion_cause_t) */
static const apt_str_table_item_t completion_cause_string_table[] = {
	{{"normal",               6},0},
	{{"barge-in",             8},0},
	{{"parse-failure",       13},0},
	{{"uri-failure",         11},0},
	{{"error",                5},0},
	{{"language-unsupported",20},4},
	{{"lexicon-load-failure",20},1},
	{{"cancelled",            9},0}
};


static APR_INLINE apr_size_t apt_string_table_value_parse(const apt_str_table_item_t *string_table, size_t count, const apt_str_t *value)
{
	return apt_string_table_id_find(string_table,count,value);
}

static apt_bool_t apt_string_table_value_generate(const apt_str_table_item_t *string_table, size_t count, size_t id, apt_text_stream_t *stream)
{
	const apt_str_t *name = apt_string_table_str_get(string_table,count,id);
	if(!name) {
		return FALSE;
	}

	memcpy(stream->pos,name->buf,name->length);
	stream->pos += name->length;
	return TRUE;
}

/** Parse MRCP speech-length value */
static apt_bool_t mrcp_speech_length_value_parse(mrcp_speech_length_value_t *speech_length, const apt_str_t *value, apr_pool_t *pool)
{
	if(!value->length) {
		return FALSE;
	}

	switch(*value->buf) {
		case '+': speech_length->type = SPEECH_LENGTH_TYPE_NUMERIC_POSITIVE; break;
		case '-': speech_length->type = SPEECH_LENGTH_TYPE_NUMERIC_NEGATIVE; break;
		default : speech_length->type = SPEECH_LENGTH_TYPE_TEXT;
	}

	if(speech_length->type == SPEECH_LENGTH_TYPE_TEXT) {
		apt_string_copy(&speech_length->value.tag,value,pool);
	}
	else {
		mrcp_numeric_speech_length_t *numeric = &speech_length->value.numeric;
		apt_str_t str;
		apt_text_stream_t stream;
		stream.text = *value;
		stream.pos = stream.text.buf;
		stream.pos++;
		if(apt_text_field_read(&stream,APT_TOKEN_SP,TRUE,&str) == FALSE) {
			return FALSE;
		}
		numeric->length = apt_size_value_parse(&str);

		if(apt_text_field_read(&stream,APT_TOKEN_SP,TRUE,&str) == FALSE) {
			return FALSE;
		}
		numeric->unit = apt_string_table_value_parse(speech_unit_string_table,SPEECH_UNIT_COUNT,&str);
	}
	return TRUE;
}

/** Generate MRCP speech-length value */
static apt_bool_t mrcp_speech_length_generate(mrcp_speech_length_value_t *speech_length, apt_text_stream_t *stream)
{
	if(speech_length->type == SPEECH_LENGTH_TYPE_TEXT) {
		apt_str_t *tag = &speech_length->value.tag;
		if(tag->length) {
			memcpy(stream->pos,tag->buf,tag->length);
			stream->pos += tag->length;
		}
	}
	else {
		if(speech_length->type == SPEECH_LENGTH_TYPE_NUMERIC_POSITIVE) {
			*stream->pos++ = '+';
		}
		else {
			*stream->pos++ = '-';
		}
		apt_size_value_generate(speech_length->value.numeric.length,stream);
		*stream->pos++ = ' ';
		apt_string_table_value_generate(speech_unit_string_table,SPEECH_UNIT_COUNT,speech_length->value.numeric.unit,stream);
	}
	return TRUE;
}

/** Generate MRCP synthesizer completion-cause */
static apt_bool_t mrcp_completion_cause_generate(mrcp_synth_completion_cause_e completion_cause, apt_text_stream_t *stream)
{
	int length;
	const apt_str_t *name = apt_string_table_str_get(completion_cause_string_table,SYNTHESIZER_COMPLETION_CAUSE_COUNT,completion_cause);
	if(!name) {
		return FALSE;
	}
	length = sprintf(stream->pos,"%03"APR_SIZE_T_FMT" ",completion_cause);
	if(length <= 0) {
		return FALSE;
	}
	stream->pos += length;

	memcpy(stream->pos,name->buf,name->length);
	stream->pos += name->length;
	return TRUE;
}

/** Initialize synthesizer header */
static void mrcp_synth_header_init(mrcp_synth_header_t *synth_header)
{
	synth_header->jump_size.type = SPEECH_LENGTH_TYPE_UNKNOWN;
	synth_header->kill_on_barge_in = FALSE;
	apt_string_reset(&synth_header->speaker_profile);
	synth_header->completion_cause = SYNTHESIZER_COMPLETION_CAUSE_UNKNOWN;
	apt_string_reset(&synth_header->completion_reason);
	synth_header->voice_param.gender = VOICE_GENDER_UNKNOWN;
	synth_header->voice_param.age = 0;
	synth_header->voice_param.variant = 0;
	apt_string_reset(&synth_header->voice_param.name);
	synth_header->prosody_param.volume = PROSODY_VOLUME_UNKNOWN;
	synth_header->prosody_param.rate = PROSODY_RATE_UNKNOWN;
	apt_string_reset(&synth_header->speech_marker);
	apt_string_reset(&synth_header->speech_language);
	apt_string_reset(&synth_header->fetch_hint);
	apt_string_reset(&synth_header->audio_fetch_hint);
	synth_header->fetch_timeout = 0;
	apt_string_reset(&synth_header->failed_uri);
	apt_string_reset(&synth_header->failed_uri_cause);
	synth_header->speak_restart = FALSE;
	synth_header->speak_length.type = SPEECH_LENGTH_TYPE_UNKNOWN;
	synth_header->load_lexicon = FALSE;
	apt_string_reset(&synth_header->lexicon_search_order);
}


/** Allocate MRCP synthesizer header */
static void* mrcp_synth_header_allocate(mrcp_header_accessor_t *accessor, apr_pool_t *pool)
{
	mrcp_synth_header_t *synth_header = apr_palloc(pool,sizeof(mrcp_synth_header_t));
	mrcp_synth_header_init(synth_header);
	accessor->data = synth_header;
	return accessor->data;
}

/** Parse MRCP synthesizer header */
static apt_bool_t mrcp_synth_header_parse(mrcp_header_accessor_t *accessor, size_t id, const apt_str_t *value, apr_pool_t *pool)
{
	apt_bool_t status = TRUE;
	mrcp_synth_header_t *synth_header = accessor->data;
	switch(id) {
		case SYNTHESIZER_HEADER_JUMP_SIZE:
			mrcp_speech_length_value_parse(&synth_header->jump_size,value,pool);
			break;
		case SYNTHESIZER_HEADER_KILL_ON_BARGE_IN:
			apt_boolean_value_parse(value,&synth_header->kill_on_barge_in);
			break;
		case SYNTHESIZER_HEADER_SPEAKER_PROFILE:
			apt_string_copy(&synth_header->speaker_profile,value,pool);
			break;
		case SYNTHESIZER_HEADER_COMPLETION_CAUSE:
			synth_header->completion_cause = apt_size_value_parse(value);
			break;
		case SYNTHESIZER_HEADER_COMPLETION_REASON:
			apt_string_copy(&synth_header->completion_reason,value,pool);
			break;
		case SYNTHESIZER_HEADER_VOICE_GENDER:
			synth_header->voice_param.gender = apt_string_table_value_parse(voice_gender_string_table,VOICE_GENDER_COUNT,value);
			break;
		case SYNTHESIZER_HEADER_VOICE_AGE:
			synth_header->voice_param.age = apt_size_value_parse(value);
			break;
		case SYNTHESIZER_HEADER_VOICE_VARIANT:
			synth_header->voice_param.variant = apt_size_value_parse(value);
			break;
		case SYNTHESIZER_HEADER_VOICE_NAME:
			apt_string_copy(&synth_header->voice_param.name,value,pool);
			break;
		case SYNTHESIZER_HEADER_PROSODY_VOLUME:
			synth_header->prosody_param.volume = apt_string_table_value_parse(prosody_volume_string_table,PROSODY_VOLUME_COUNT,value);
			break;
		case SYNTHESIZER_HEADER_PROSODY_RATE:
			synth_header->prosody_param.rate = apt_string_table_value_parse(prosody_rate_string_table,PROSODY_RATE_COUNT,value);
			break;
		case SYNTHESIZER_HEADER_SPEECH_MARKER:
			apt_string_copy(&synth_header->speech_marker,value,pool);
			break;
		case SYNTHESIZER_HEADER_SPEECH_LANGUAGE:
			apt_string_copy(&synth_header->speech_language,value,pool);
			break;
		case SYNTHESIZER_HEADER_FETCH_HINT:
			apt_string_copy(&synth_header->fetch_hint,value,pool);
			break;
		case SYNTHESIZER_HEADER_AUDIO_FETCH_HINT:
			apt_string_copy(&synth_header->audio_fetch_hint,value,pool);
			break;
		case SYNTHESIZER_HEADER_FETCH_TIMEOUT:
			synth_header->fetch_timeout = apt_size_value_parse(value);
			break;
		case SYNTHESIZER_HEADER_FAILED_URI:
			apt_string_copy(&synth_header->failed_uri,value,pool);
			break;
		case SYNTHESIZER_HEADER_FAILED_URI_CAUSE:
			apt_string_copy(&synth_header->failed_uri_cause,value,pool);
			break;
		case SYNTHESIZER_HEADER_SPEAK_RESTART:
			apt_boolean_value_parse(value,&synth_header->speak_restart);
			break;
		case SYNTHESIZER_HEADER_SPEAK_LENGTH:
			mrcp_speech_length_value_parse(&synth_header->speak_length,value,pool);
			break;
		case SYNTHESIZER_HEADER_LOAD_LEXICON:
			apt_boolean_value_parse(value,&synth_header->load_lexicon);
			break;
		case SYNTHESIZER_HEADER_LEXICON_SEARCH_ORDER:
			apt_string_copy(&synth_header->lexicon_search_order,value,pool);
			break;
		default:
			status = FALSE;
	}
	return status;
}

/** Generate MRCP synthesizer header */
static apt_bool_t mrcp_synth_header_generate(mrcp_header_accessor_t *accessor, size_t id, apt_text_stream_t *value)
{
	mrcp_synth_header_t *synth_header = accessor->data;
	switch(id) {
		case SYNTHESIZER_HEADER_JUMP_SIZE:
			mrcp_speech_length_generate(&synth_header->jump_size,value);
			break;
		case SYNTHESIZER_HEADER_KILL_ON_BARGE_IN:
			apt_boolean_value_generate(synth_header->kill_on_barge_in,value);
			break;
		case SYNTHESIZER_HEADER_SPEAKER_PROFILE:
			apt_string_value_generate(&synth_header->speaker_profile,value);
			break;
		case SYNTHESIZER_HEADER_COMPLETION_CAUSE:
			mrcp_completion_cause_generate(synth_header->completion_cause,value);
			break;
		case SYNTHESIZER_HEADER_COMPLETION_REASON:
			apt_string_value_generate(&synth_header->completion_reason,value);
			break;
		case SYNTHESIZER_HEADER_VOICE_GENDER:
			apt_string_table_value_generate(voice_gender_string_table,VOICE_GENDER_COUNT,synth_header->voice_param.gender,value);
			break;
		case SYNTHESIZER_HEADER_VOICE_AGE:
			apt_size_value_generate(synth_header->voice_param.age,value);
			break;
		case SYNTHESIZER_HEADER_VOICE_VARIANT:
			apt_size_value_generate(synth_header->voice_param.variant,value);
			break;
		case SYNTHESIZER_HEADER_VOICE_NAME:
			apt_string_value_generate(&synth_header->voice_param.name,value);
			break;
		case SYNTHESIZER_HEADER_PROSODY_VOLUME:
			apt_string_table_value_generate(prosody_volume_string_table,PROSODY_VOLUME_COUNT,synth_header->prosody_param.volume,value);
			break;
		case SYNTHESIZER_HEADER_PROSODY_RATE:
			apt_string_table_value_generate(prosody_rate_string_table,PROSODY_RATE_COUNT,synth_header->prosody_param.rate,value);
			break;
		case SYNTHESIZER_HEADER_SPEECH_MARKER:
			apt_string_value_generate(&synth_header->speech_marker,value);
			break;
		case SYNTHESIZER_HEADER_SPEECH_LANGUAGE:
			apt_string_value_generate(&synth_header->speech_language,value);
			break;
		case SYNTHESIZER_HEADER_FETCH_HINT:
			apt_string_value_generate(&synth_header->fetch_hint,value);
			break;
		case SYNTHESIZER_HEADER_AUDIO_FETCH_HINT:
			apt_string_value_generate(&synth_header->audio_fetch_hint,value);
			break;
		case SYNTHESIZER_HEADER_FETCH_TIMEOUT:
			apt_size_value_generate(synth_header->fetch_timeout,value);
		case SYNTHESIZER_HEADER_FAILED_URI:
			apt_string_value_generate(&synth_header->failed_uri,value);
			break;
		case SYNTHESIZER_HEADER_FAILED_URI_CAUSE:
			apt_string_value_generate(&synth_header->failed_uri_cause,value);
			break;
		case SYNTHESIZER_HEADER_SPEAK_RESTART:
			apt_boolean_value_generate(synth_header->speak_restart,value);
			break;
		case SYNTHESIZER_HEADER_SPEAK_LENGTH:
			mrcp_speech_length_generate(&synth_header->speak_length,value);
			break;
		case SYNTHESIZER_HEADER_LOAD_LEXICON:
			apt_boolean_value_generate(synth_header->load_lexicon,value);
			break;
		case SYNTHESIZER_HEADER_LEXICON_SEARCH_ORDER:
			apt_string_value_generate(&synth_header->lexicon_search_order,value);
			break;
		default:
			break;
	}
	return TRUE;
}

/** Duplicate MRCP synthesizer header */
static apt_bool_t mrcp_synth_header_duplicate(mrcp_header_accessor_t *accessor, const mrcp_header_accessor_t *src, size_t id, apr_pool_t *pool)
{
	mrcp_synth_header_t *synth_header = accessor->data;
	const mrcp_synth_header_t *src_synth_header = src->data;
	apt_bool_t status = TRUE;

	if(!synth_header || !src_synth_header) {
		return FALSE;
	}

	switch(id) {
		case SYNTHESIZER_HEADER_JUMP_SIZE:
			synth_header->jump_size = src_synth_header->jump_size;
			break;
		case SYNTHESIZER_HEADER_KILL_ON_BARGE_IN:
			synth_header->kill_on_barge_in = src_synth_header->kill_on_barge_in;
			break;
		case SYNTHESIZER_HEADER_SPEAKER_PROFILE:
			apt_string_copy(&synth_header->speaker_profile,&src_synth_header->speaker_profile,pool);
			break;
		case SYNTHESIZER_HEADER_COMPLETION_CAUSE:
			synth_header->completion_cause = src_synth_header->completion_cause;
			break;
		case SYNTHESIZER_HEADER_COMPLETION_REASON:
			apt_string_copy(&synth_header->completion_reason,&src_synth_header->completion_reason,pool);
			break;
		case SYNTHESIZER_HEADER_VOICE_GENDER:
			synth_header->voice_param.gender = src_synth_header->voice_param.gender;
			break;
		case SYNTHESIZER_HEADER_VOICE_AGE:
			synth_header->voice_param.age = src_synth_header->voice_param.age;
			break;
		case SYNTHESIZER_HEADER_VOICE_VARIANT:
			synth_header->voice_param.variant = src_synth_header->voice_param.variant;
			break;
		case SYNTHESIZER_HEADER_VOICE_NAME:
			apt_string_copy(&synth_header->voice_param.name,&src_synth_header->voice_param.name,pool);
			break;
		case SYNTHESIZER_HEADER_PROSODY_VOLUME:
			synth_header->prosody_param.volume = src_synth_header->prosody_param.volume;
			break;
		case SYNTHESIZER_HEADER_PROSODY_RATE:
			synth_header->prosody_param.rate = src_synth_header->prosody_param.rate;
			break;
		case SYNTHESIZER_HEADER_SPEECH_MARKER:
			apt_string_copy(&synth_header->speech_marker,&src_synth_header->speech_marker,pool);
			break;
		case SYNTHESIZER_HEADER_SPEECH_LANGUAGE:
			apt_string_copy(&synth_header->speech_language,&src_synth_header->speech_language,pool);
			break;
		case SYNTHESIZER_HEADER_FETCH_HINT:
			apt_string_copy(&synth_header->fetch_hint,&src_synth_header->fetch_hint,pool);
			break;
		case SYNTHESIZER_HEADER_AUDIO_FETCH_HINT:
			apt_string_copy(&synth_header->audio_fetch_hint,&src_synth_header->audio_fetch_hint,pool);
			break;
		case SYNTHESIZER_HEADER_FETCH_TIMEOUT:
			synth_header->fetch_timeout = src_synth_header->fetch_timeout;
			break;
		case SYNTHESIZER_HEADER_FAILED_URI:
			apt_string_copy(&synth_header->failed_uri,&src_synth_header->failed_uri,pool);
			break;
		case SYNTHESIZER_HEADER_FAILED_URI_CAUSE:
			apt_string_copy(&synth_header->failed_uri_cause,&src_synth_header->failed_uri_cause,pool);
			break;
		case SYNTHESIZER_HEADER_SPEAK_RESTART:
			synth_header->speak_restart = src_synth_header->speak_restart;
			break;
		case SYNTHESIZER_HEADER_SPEAK_LENGTH:
			synth_header->speak_length = src_synth_header->speak_length;
			break;
		case SYNTHESIZER_HEADER_LOAD_LEXICON:
			synth_header->load_lexicon = src_synth_header->load_lexicon;
			break;
		case SYNTHESIZER_HEADER_LEXICON_SEARCH_ORDER:
			apt_string_copy(&synth_header->lexicon_search_order,&src_synth_header->lexicon_search_order,pool);
			break;
		default:
			status = FALSE;
	}
	return status;
}

static const mrcp_header_vtable_t vtable = {
	mrcp_synth_header_allocate,
	NULL, /* nothing to destroy */
	mrcp_synth_header_parse,
	mrcp_synth_header_generate,
	mrcp_synth_header_duplicate,
	synth_header_string_table,
	SYNTHESIZER_HEADER_COUNT
};

MRCP_DECLARE(const mrcp_header_vtable_t*) mrcp_synth_header_vtable_get(mrcp_version_e version)
{
	return &vtable;
}
