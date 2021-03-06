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

#include <stdlib.h>
#include "mpf_codec_manager.h"
#include "apt_log.h"


struct mpf_codec_manager_t {
	/** Memory pool */
	apr_pool_t   *pool;

	/** Dynamic array of codecs (mpf_codec_t*) */
	apr_array_header_t *codec_arr;
};


mpf_codec_descriptor_t* mpf_codec_lpcm_descriptor_create(apr_uint16_t sampling_rate, apr_byte_t channel_count, apr_pool_t *pool);

MPF_DECLARE(mpf_codec_manager_t*) mpf_codec_manager_create(apr_size_t codec_count, apr_pool_t *pool)
{
	mpf_codec_manager_t *codec_manager = apr_palloc(pool,sizeof(mpf_codec_manager_t));
	codec_manager->pool = pool;
	codec_manager->codec_arr = apr_array_make(pool,(int)codec_count,sizeof(mpf_codec_t*));
	return codec_manager;
}

MPF_DECLARE(void) mpf_codec_manager_destroy(mpf_codec_manager_t *codec_manager)
{
	/* nothing to do */
}

MPF_DECLARE(apt_bool_t) mpf_codec_manager_codec_register(mpf_codec_manager_t *codec_manager, mpf_codec_t *codec)
{
	mpf_codec_t **slot;
	if(!codec || !codec->attribs || !codec->attribs->name.buf) {
		return FALSE;
	}

	apt_log(APT_LOG_MARK,APT_PRIO_INFO,"Register Codec [%s]",codec->attribs->name.buf);

	slot = apr_array_push(codec_manager->codec_arr);
	*slot = codec;
	return TRUE;
}

MPF_DECLARE(mpf_codec_t*) mpf_codec_manager_codec_get(const mpf_codec_manager_t *codec_manager, mpf_codec_descriptor_t *descriptor, apr_pool_t *pool)
{
	int i;
	mpf_codec_t *codec = NULL;
	mpf_codec_t *ret_codec = NULL;
	if(!descriptor) {
		return NULL;
	}

	for(i=0; i<codec_manager->codec_arr->nelts; i++) {
		codec = ((mpf_codec_t**)codec_manager->codec_arr->elts)[i];
		if(descriptor->payload_type < 96) {
			if(codec->static_descriptor && codec->static_descriptor->payload_type == descriptor->payload_type) {
				descriptor->name = codec->static_descriptor->name;
				descriptor->sampling_rate = codec->static_descriptor->sampling_rate;
				descriptor->channel_count = codec->static_descriptor->channel_count;
				break;
			}
		}
		else {
			if(apt_string_compare(&codec->attribs->name,&descriptor->name) == TRUE) {
				/* sampling rate must be checked as well */
				break;
			}
		}
	}

	if(i == codec_manager->codec_arr->nelts) {
		/* no match found */
		return NULL;
	}
	if(codec) {
		ret_codec = mpf_codec_clone(codec,pool);
		ret_codec->descriptor = descriptor;
	}
	return ret_codec;
}

MPF_DECLARE(mpf_codec_t*) mpf_codec_manager_default_codec_get(const mpf_codec_manager_t *codec_manager, apr_pool_t *pool)
{
	mpf_codec_t *codec;
	mpf_codec_descriptor_t *descriptor = mpf_codec_lpcm_descriptor_create(8000,1,pool);
	codec = mpf_codec_manager_codec_get(codec_manager,descriptor,pool);
	return codec;
}

MPF_DECLARE(apt_bool_t) mpf_codec_manager_codec_list_get(const mpf_codec_manager_t *codec_manager, mpf_codec_list_t *codec_list, apr_pool_t *pool)
{
	const mpf_codec_descriptor_t *static_descriptor;
	mpf_codec_descriptor_t *descriptor;
	int i;
	mpf_codec_t *codec;

	mpf_codec_list_init(codec_list,codec_manager->codec_arr->nelts,pool);
	for(i=0; i<codec_manager->codec_arr->nelts; i++) {
		codec = ((mpf_codec_t**)codec_manager->codec_arr->elts)[i];
		static_descriptor = codec->static_descriptor;
		if(static_descriptor) {
			descriptor = mpf_codec_list_add(codec_list);
			if(descriptor) {
				*descriptor = *static_descriptor;
			}
		}
	}
	return TRUE;
}

static apt_bool_t mpf_codec_manager_codec_parse(const mpf_codec_manager_t *codec_manager, mpf_codec_list_t *codec_list, char *codec_desc_str, apr_pool_t *pool)
{
	const mpf_codec_t *codec;
	mpf_codec_descriptor_t *descriptor;
	const char *separator = "/";
	char *state;
	/* parse codec name */
	char *str = apr_strtok(codec_desc_str, separator, &state);
	codec_desc_str = NULL; /* make sure we pass NULL on subsequent calls of apr_strtok() */
	if(str) {
		apt_str_t name;
		apt_string_assign(&name,str,pool);
		/* find codec by name */
		codec = mpf_codec_manager_codec_find(codec_manager,&name);
		if(!codec) {
			apt_log(APT_LOG_MARK,APT_PRIO_WARNING,"No Such Codec [%s]",str);
			return FALSE;
		}

		descriptor = mpf_codec_list_add(codec_list);
		descriptor->name = name;

		/* set defualt attributes */
		if(codec->static_descriptor) {
			descriptor->payload_type = codec->static_descriptor->payload_type;
			descriptor->sampling_rate = codec->static_descriptor->sampling_rate;
			descriptor->channel_count = codec->static_descriptor->channel_count;
		}
		else {
			descriptor->payload_type = 96;
			descriptor->sampling_rate = 8000;
			descriptor->channel_count = 1;
		}

		/* parse optional payload type */
		str = apr_strtok(codec_desc_str, separator, &state);
		if(str) {
			descriptor->payload_type = (apr_byte_t)atol(str);

			/* parse optional sampling rate */
			str = apr_strtok(codec_desc_str, separator, &state);
			if(str) {
				descriptor->sampling_rate = (apr_uint16_t)atol(str);

				/* parse optional channel count */
				str = apr_strtok(codec_desc_str, separator, &state);
				if(str) {
					descriptor->channel_count = (apr_byte_t)atol(str);
				}
			}
		}
	}
	return TRUE;
}

MPF_DECLARE(apt_bool_t) mpf_codec_manager_codec_list_load(const mpf_codec_manager_t *codec_manager, mpf_codec_list_t *codec_list, const char *str, apr_pool_t *pool)
{
	char *codec_desc_str;
	char *state;
	char *codec_list_str = apr_pstrdup(pool,str);
	do {
		codec_desc_str = apr_strtok(codec_list_str, " ", &state);
		if(codec_desc_str) {
			mpf_codec_manager_codec_parse(codec_manager,codec_list,codec_desc_str,pool);
		}
		codec_list_str = NULL; /* make sure we pass NULL on subsequent calls of apr_strtok() */
	} 
	while(codec_desc_str);
	return TRUE;
}

MPF_DECLARE(const mpf_codec_t*) mpf_codec_manager_codec_find(const mpf_codec_manager_t *codec_manager, const apt_str_t *codec_name)
{
	int i;
	mpf_codec_t *codec;
	for(i=0; i<codec_manager->codec_arr->nelts; i++) {
		codec = ((mpf_codec_t**)codec_manager->codec_arr->elts)[i];
		if(apt_string_compare(&codec->attribs->name,codec_name) == TRUE) {
			return codec;
		}
	}
	return NULL;
}
