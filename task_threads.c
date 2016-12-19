#include <task_threads.h>
#include <telnet.h>

void* thread_new_client_connection(void *args)
{
	client_t temp_client;
	char msg[BUFFER_LEN];
	char timestamp[TIMESTAMP_LEN];

	/* get resources from args */
	resources_t *r = (resources_t*) args;

	/* accept new connection request */
	if (server_accept(r->server, &temp_client) != 0)
	{
		return (void *) -1;
	}

	/* if there is already a new client request being handled then reject this one */
	if (r->new_client->socket != -1)
	{
		sprintf(msg, "\nToo many connection requests, please try later.\n");
		send(temp_client.socket, msg, strlen(msg), 0);

		client_close(&temp_client);

		time2string(time(NULL), timestamp);
		LOG("rejected new client request %s @ %s", temp_client.ip_string, timestamp);

		return (void *) 0;
	}
	/* otherwise the next step depends on the status of the current client */
	else
	{
		/* if no client is connected then immediately accept the new client */
		if (r->client->socket == -1)
		{
			memcpy(r->new_client, &temp_client, sizeof(client_t));
			return (void *) 0;
		}
		else
		{
			/* Reaching this point means there is already a connected client
			 * but this request could be accepted. Ask the new client if the
			 * current client connection should be dropped. */

			/* inform the new client that the port is already in use */
			time2string(r->client->last_active, timestamp);
			sprintf(msg, "\nPort %u is already being used!\n"
					"Current user and last activity:\n%s @ %s\n",
					r->server->port, r->client->username, timestamp);
			send(temp_client.socket, msg, strlen(msg), 0);

			/* ask the new client if the current client should be dropped */
			sprintf(msg, "\nDo you want to drop the current user?\n"
					"If yes then please type YES DROP (in uppercase):\n");
			send(temp_client.socket, msg, strlen(msg), 0);

			/* wait for new client input */
			client_wait_line(&temp_client);

			/* check new client confirmation */
			if (strncmp(temp_client.data, "YES DROP", 8) == 0)
			{
				/* drop the currently connected client */
				client_close(r->client);
				/* accept the new client */
				memcpy(r->new_client, &temp_client, sizeof(client_t));

				LOG("dropped client %s @ %s", r->client->ip_string, timestamp);
			}
			else
			{
				/* reject this client request */
				client_close(&temp_client);

				time2string(time(NULL), timestamp);
				LOG("rejected new client request %s @ %s", temp_client.ip_string, timestamp);

				return (void *) 1;
			}
		}
	}

	return (void *) 0;
}

void* thread_tty_data(void *args)
{
	struct timeval tv;
	fd_set read_fds;
	int ret;

	/* get resources from args */
	resources_t *r = (resources_t*) args;

	LOG("tty thread started with device: %s", r->tty_dev->path);

	/* loop with timeouts waiting for data from the tty device */
	while (1)
	{
		/* set parameters for select() */
		tv.tv_sec = TTY_WAIT_TIMEOUT;
		tv.tv_usec = 0;
		FD_ZERO(&read_fds);
		FD_SET(r->tty_dev->fd, &read_fds);

		/* wait with select() */
		ret = select(r->tty_dev->fd + 1, &read_fds, NULL, NULL, &tv);

		if ( (ret > 0) && (FD_ISSET(r->tty_dev->fd, &read_fds)) )
		{
			/* pass data from tty device to client */
			ret = tty_read(r->tty_dev);
			client_write(r->client, r->tty_dev->data, ret);
		}

		if (debug_messages)
		{
			LOG("tty thread alive");
		}
	} /* end main while() loop */

	LOG("tty thread stopped");

	return (void*) 0;
}

void* thread_client_data(void *args)
{
	struct timeval tv;
	fd_set read_fds;
	int fdmax;
	int ret;
	pthread_t new_client_thread;

	/* get resources from args */
	resources_t *r = (resources_t*) args;

	/* loop with timeouts waiting for client data or new connection requests */
	while (1)
	{
		/* check if there is no connected client, but a new client is available */
		if ( (r->client->socket == -1) && (r->new_client->socket != -1) )
		{
			/* copy new client information */
			memcpy(r->client, r->new_client, sizeof(client_t));
			r->new_client->socket = -1;
			LOG("client %s connected", r->client->ip_string);
			/* ask client to provide a username before going to "character" mode */
			if (client_ask_username(r->client) != 0)
			{
				/* close client if not able to provide a username */
				client_close(r->client);
				continue;
			}
			/* put client in "character" mode */
			char msg[TELNET_MSG_LEN_CHARMODE];
			telnet_message_set_character_mode(msg);
			client_write(r->client, msg, TELNET_MSG_LEN_CHARMODE);
		}

		/* setup parameters for select() */
		tv.tv_sec = SERVER_WAIT_TIMEOUT;
		tv.tv_usec = 0;
		FD_ZERO(&read_fds);
		/* always wait for new connections on server socket */
		FD_SET(r->server->socket, &read_fds);
		/* wait for client only if connected */
		if (r->client->socket != -1)
		{
			FD_SET(r->client->socket, &read_fds);
		}
		fdmax = (r->server->socket > r->client->socket) ? r->server->socket : r->client->socket;

		/* wait with select() */
		ret = select(fdmax+1, &read_fds, NULL, NULL, &tv);
		/* handle errors from select() */
		if (ret == -1)
		{
			//TODO do we really break here and stop server when select returns an error?
			LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
			break;
		}
		/* handle incoming data on server and client sockets */
		if (ret > 0)
		{
			/* check for new connection requests */
			if (FD_ISSET(r->server->socket, &read_fds))
			{
				LOG("received client connection request");
				/* handle new client connection request in a separate thread */
				if (pthread_create(&new_client_thread, NULL, thread_new_client_connection, r) != 0)
				{
					/* print error but continue waiting for connection requests */
					LOG("problem with handling client connection request");
					continue;
				}
			}
			/* check client status only if connected */
			if ( (r->client->socket != -1) && FD_ISSET(r->client->socket, &read_fds) )
			{
				/* read client data */
				ret = client_read(r->client);
				/* check if client disconnected */
				if (ret == -ENODATA)
				{
					LOG("client %s disconnected", r->client->ip_string);
					/* close client connection and continue waiting for new clients */
					client_close(r->client);
					continue;
				}
				/* ignore read errors */
				if (ret < 0)
				{
					/* print error but continue waiting for new data */
					LOG("problem reading from client, but will continue");
					continue;
				}
				/* otherwise, pass received client data to the tty device */
				else
				{
					if (r->tty_dev->fd != -1)
					{
						tty_write(r->tty_dev, r->client->data, ret);
					}
				}
			}
		}
		/* handle timeout from select() */
		if (ret == 0)
		{
			/* do something with inactive client */
			if (r->client->socket != -1)
			{
				//TODO we could drop client if inactive for some time
				time_t current_time = time(NULL);
				if (debug_messages)
				{
					LOG("client last active %u seconds ago",
						(unsigned int) (current_time - r->client->last_active));
				}
			}
			/* do something while listening for client connections? */
			else
			{
				if (debug_messages)
				{
					LOG("listening for client connection");
				}
			}
		}
	} /* end main while() loop */

	return (void*) 0;
}
