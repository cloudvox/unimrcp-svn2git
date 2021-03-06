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

#ifndef __MRCP_SOFIASIP_CLIENT_AGENT_H__
#define __MRCP_SOFIASIP_CLIENT_AGENT_H__

/**
 * @file mrcp_sofiasip_client_agent.h
 * @brief Implementation of MRCP Signaling Interface using Sofia-SIP
 */ 

#include <apr_network_io.h>
#include "mrcp_sig_agent.h"

APT_BEGIN_EXTERN_C

/** Sofia-SIP config declaration */
typedef struct mrcp_sofia_client_config_t mrcp_sofia_client_config_t;

/** Sofia-SIP config */
struct mrcp_sofia_client_config_t {
	/** Local IP address */
	char      *local_ip;
	/** Local SIP port */
	apr_port_t local_port;
	/** Local SIP user name */
	char      *local_user_name;
	
	/** Remote IP address */
	char      *remote_ip;
	/** Remote SIP port */
	apr_port_t remote_port;
	/** Remote SIP user name */
	char      *remote_user_name;

	/** User agent name */
	char      *user_agent_name;
	/** SDP origin */
	char      *origin;
	/** SIP transport */
	char      *transport;
};

/**
 * Create Sofia-SIP signaling agent.
 */
MRCP_DECLARE(mrcp_sig_agent_t*) mrcp_sofiasip_client_agent_create(mrcp_sofia_client_config_t *config, apr_pool_t *pool);

/**
 * Allocate Sofia-SIP config.
 */
MRCP_DECLARE(mrcp_sofia_client_config_t*) mrcp_sofiasip_client_config_alloc(apr_pool_t *pool);

APT_END_EXTERN_C

#endif /*__MRCP_SOFIASIP_CLIENT_AGENT_H__*/
