#include <client.h>
#include <telnet.h>

void client_close(client_t *client)
{
	char timestamp[TIMESTAMP_LEN];

	/* force closing in case of error */
	if (close(client->socket) == -1)
	{
		close(client->socket);
	}
	client->socket = -1;

	time2string(time(NULL), timestamp);
	LOG("socket closed for client %s @ %s", client->ip_string, timestamp);
}

int client_read(client_t *client)
{
	int len;
	
	/* read data from the client */
	len = recv(client->socket, client->data, sizeof(client->data)-1, 0);
	if (len == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
		return -errno;
	}
	/* a disconnected client socket is ready for reading but read returns 0 */
	if (len == 0)
	{
		LOG("[@%d] no data available from client", __LINE__);
		return -ENODATA;
	}
	
	//TODO let's print received bytes during development phase...
	if (debug_messages)
	{
		int i;
		for(i = 0; i < len; i++)
		{
			LOG("client %s <- %u '%c'",
				client->ip_string,
				(unsigned char) client->data[i],
				(unsigned char) client->data[i]);
		}	
	}
	
	/* handle special telnet characters coming from the client */
	telnet_filter_client_read(client->data, &len);
	
	/* grab current time and store it as client's last activity */
	client->last_active = time(NULL);
	
	return len;
}

int client_write(client_t *client, char *databuf, int datalen)
{
	int len;
	
	/* handle special telnet characters to display them correctly on client */
	//telnet_filter_client_write(databuf, &datalen);
	
	//TODO let's print received bytes during development phase...
	if (debug_messages)
	{
		int i;
		for(i = 0; i < datalen; i++)
		{
			LOG("client %s -> %u '%c'",
				client->ip_string,
				(unsigned char) databuf[i],
				(unsigned char) databuf[i]);
		}	
	}
	
	/* send data to the client */
	len = send(client->socket, databuf, datalen, 0);
	if (len == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__,	errno, strerror(errno));
		return -errno;
	}
	
	return len;
}

int client_wait_line(client_t *client)
{
	fd_set read_fds;
	struct timeval tv;
	
	client->data[0] = '\0';
	/* loop waiting for client input */
	while (client->data[0] == '\0')
	{
		/* setup select() parameters */
		tv.tv_sec = 15; /* 15 second timeout */
		tv.tv_usec = 0;
		FD_ZERO(&read_fds);
		FD_SET(client->socket, &read_fds);
		/* send prompt character to the client */
		client_write(client, "> ", 2);
		/* block until input arrives */
		if (select((client->socket)+1, &read_fds, NULL, NULL, &tv) <= 0)
		{
			return -1;
		}
		if (FD_ISSET(client->socket, &read_fds))
		{
			/* read client input */
			if (client_read(client) == -1)
			{
				return -1;
			}
			/* we don't want empty data so ignore data starting with \r or \n */
			if ( (client->data[0] == '\r') || (client->data[0] == '\n') )
			{
				client->data[0] = '\0';
			}
		}
	}

	return 0;
}

int client_ask_username(client_t *client)
{
	int i;
	char msg[BUFFER_LEN];
	
	/* show username request to the client */
	snprintf(msg, BUFFER_LEN,
			 "\nPlease provide a username to identify yourself to "
			 "other users (max %d characters):\n", USERNAME_LEN);
	client_write(client, msg, strlen(msg));
	
	/* wait for client input */
	if (client_wait_line(client) != 0)
	{
		return -1;
	}
	
	/* save received data as client username */
	for (i = 0; i < USERNAME_LEN; i++)
	{
		if ( (client->data[i] == '\r') || (client->data[i] == '\n') )
		{
			/* don't include \r or \n in the username */
			client->username[i] = '\0';
			break;
		}
		client->username[i] = client->data[i];
	}
	
	/* show welcome message to client */
	snprintf(msg, BUFFER_LEN,
			 "\nWelcome %s!\n\n", client->username);
	client_write(client, msg, strlen(msg));
	
	return 0;
}
