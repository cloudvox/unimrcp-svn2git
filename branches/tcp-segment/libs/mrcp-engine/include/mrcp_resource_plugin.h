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

#ifndef __MRCP_RESOURCE_PLUGIN_H__
#define __MRCP_RESOURCE_PLUGIN_H__

/**
 * @file mrcp_resource_plugin.h
 * @brief MRCP Resource Engine Plugin
 */ 

#include "apr_version.h"

APT_BEGIN_EXTERN_C

/** Plugin export defines */
#ifdef WIN32
#define MRCP_PLUGIN_DECLARE(type) EXTERN_C __declspec(dllexport) type
#else
#define MRCP_PLUGIN_DECLARE(type) type
#endif

/** Symbol name of the entry point in plugin DSO */
#define MRCP_PLUGIN_SYM_NAME "mrcp_plugin_create"

/** MRCP resource engine declaration */
typedef struct mrcp_resource_engine_t mrcp_resource_engine_t;
/** Prototype of resource engine creator (entry point of plugin DSO) */
typedef mrcp_resource_engine_t* (*mrcp_plugin_creator_f)(apr_pool_t *pool);

/** major version 
 * Major API changes that could cause compatibility problems for older
 * plugins such as structure size changes.  No binary compatibility is
 * possible across a change in the major version.
 */
#define PLUGIN_MAJOR_VERSION   0

/** minor version
 * Minor API changes that do not cause binary compatibility problems.
 * Reset to 0 when upgrading PLUGIN_MAJOR_VERSION
 */
#define PLUGIN_MINOR_VERSION   2

/** patch level 
 * The Patch Level never includes API changes, simply bug fixes.
 * Reset to 0 when upgrading PLUGIN_MINOR_VERSION
 */
#define PLUGIN_PATCH_VERSION   0


/**
 * Check at compile time if the plugin version is at least a certain
 * level.
 */
#define PLUGIN_VERSION_AT_LEAST(major,minor,patch)                    \
(((major) < PLUGIN_MAJOR_VERSION)                                     \
 || ((major) == PLUGIN_MAJOR_VERSION && (minor) < PLUGIN_MINOR_VERSION) \
 || ((major) == PLUGIN_MAJOR_VERSION && (minor) == PLUGIN_MINOR_VERSION && (patch) <= PLUGIN_PATCH_VERSION))

/** The formatted string of plugin's version */
#define PLUGIN_VERSION_STRING \
     APR_STRINGIFY(PLUGIN_MAJOR_VERSION) "." \
     APR_STRINGIFY(PLUGIN_MINOR_VERSION) "." \
     APR_STRINGIFY(PLUGIN_PATCH_VERSION)

/** Plugin version */
typedef apr_version_t mrcp_plugin_version_t;

/** Get plugin version */
static APR_INLINE void mrcp_plugin_version_get(mrcp_plugin_version_t *version)
{
	version->major = PLUGIN_MAJOR_VERSION;
	version->minor = PLUGIN_MINOR_VERSION;
	version->patch = PLUGIN_PATCH_VERSION;
}

/** Check plugin version */
static APR_INLINE int mrcp_plugin_version_check(mrcp_plugin_version_t *version)
{
	return PLUGIN_VERSION_AT_LEAST(version->major,version->minor,version->patch);
}

APT_END_EXTERN_C

#endif /*__MRCP_RESOURCE_PLUGIN_H__*/
