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

#ifndef __MRCP_CONNECTION_H__
#define __MRCP_CONNECTION_H__

/**
 * @file mrcp_connection.h
 * @brief MRCP Connection
 */ 

#include <apr_poll.h>
#include <apr_hash.h>
#include "apt_obj_list.h"
#include "mrcp_connection_types.h"

APT_BEGIN_EXTERN_C

/** MRCPv2 connection */
struct mrcp_connection_t {
	/** Memory pool */
	apr_pool_t      *pool;

	/** Accepted/Connected socket */
	apr_socket_t    *sock;
	/** Socket poll descriptor */
	apr_pollfd_t     sock_pfd;
	/** Remote sockaddr */
	apr_sockaddr_t  *sockaddr;
	/** Remote IP */
	apt_str_t        remote_ip;

	/** Reference count */
	apr_size_t       access_count;
	/** Agent list element */
	apt_list_elem_t *it;

	/** Table of control channels */
	apr_hash_t      *channel_table;
};

/** Create MRCP connection. */
mrcp_connection_t* mrcp_connection_create();

/** Destroy MRCP connection. */
void mrcp_connection_destroy(mrcp_connection_t *connection);

/** Add Control Channel to MRCP connection. */
apt_bool_t mrcp_connection_channel_add(mrcp_connection_t *connection, mrcp_control_channel_t *channel);

/** Find Control Channel by Channel Identifier. */
mrcp_control_channel_t* mrcp_connection_channel_find(mrcp_connection_t *connection, const apt_str_t *identifier);

/** Remove Control Channel from MRCP connection. */
apt_bool_t mrcp_connection_channel_remove(mrcp_connection_t *connection, mrcp_control_channel_t *channel);


APT_END_EXTERN_C

#endif /*__MRCP_CONNECTION_H__*/
