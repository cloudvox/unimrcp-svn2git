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

#include "mrcp_connection.h"

mrcp_connection_t* mrcp_connection_create()
{
	mrcp_connection_t *connection;
	apr_pool_t *pool;
	if(apr_pool_create(&pool,NULL) != APR_SUCCESS) {
		return NULL;
	}
	
	connection = apr_palloc(pool,sizeof(mrcp_connection_t));
	connection->pool = pool;
	apt_string_reset(&connection->remote_ip);
	connection->sockaddr = NULL;
	connection->sock = NULL;
	connection->access_count = 0;
	connection->it = NULL;
	connection->channel_table = apr_hash_make(pool);
	apt_text_stream_init(&connection->rx_stream,connection->rx_buffer,sizeof(connection->rx_buffer)-1);
	apt_text_stream_init(&connection->tx_stream,connection->tx_buffer,sizeof(connection->tx_buffer)-1);
	connection->parser = NULL;
	connection->generator = NULL;
	return connection;
}

void mrcp_connection_destroy(mrcp_connection_t *connection)
{
	if(connection && connection->pool) {
		apr_pool_destroy(connection->pool);
	}
}

apt_bool_t mrcp_connection_channel_add(mrcp_connection_t *connection, mrcp_control_channel_t *channel)
{
	if(!connection || !channel) {
		return FALSE;
	}
	apr_hash_set(connection->channel_table,channel->identifier.buf,channel->identifier.length,channel);
	channel->connection = connection;
	connection->access_count++;
	return TRUE;
}

mrcp_control_channel_t* mrcp_connection_channel_find(mrcp_connection_t *connection, const apt_str_t *identifier)
{
	if(!connection || !identifier) {
		return NULL;
	}
	return apr_hash_get(connection->channel_table,identifier->buf,identifier->length);
}

apt_bool_t mrcp_connection_channel_remove(mrcp_connection_t *connection, mrcp_control_channel_t *channel)
{
	if(!connection || !channel) {
		return FALSE;
	}
	apr_hash_set(connection->channel_table,channel->identifier.buf,channel->identifier.length,NULL);
	channel->connection = NULL;
	connection->access_count--;
	return TRUE;
}
