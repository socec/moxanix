/* 
 * Handling communication with clients.
 */

#include "moxerver.h"

/* Closes client connection. */
int client_close(struct client_t *client) {
	char timestamp[TIMESTAMP_LEN];
	/* force closing in case of error */
	if (close(client->socket) == -1) {
		close(client->socket);
	}
	client->socket = -1;
	time2string(time(NULL), timestamp);
	fprintf(stderr,"[%s] socket closed for client %s @ %s\n", __func__, client->ip_string, timestamp);
	return 0;
}

/* Reads data from client into client data buffer. Returns number of read bytes. */
int client_read(struct client_t *client) {
	
	int len;
	
	/* read client data */
	len = recv(client->socket, client->data, sizeof(client->data)-1, 0);
	if (len == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	/* a disconnected client socket is ready for reading but read returns 0 */
	if (len == 0) {
		fprintf(stderr, "[%s:%d] no data available from client\n", __func__, __LINE__);
		return -ENODATA;
	}
	
	//TODO let's print received bytes during development phase...
	if (debug_messages) {
		int i;
		for(i = 0; i < len; i++) {
			fprintf(stderr, "client %s <- %u '%c'\n",
					client->ip_string,
					(unsigned char) client->data[i],
					(unsigned char) client->data[i]);
		}	
	}
	
	/* handle special telnet characters coming from client */
	telnet_handle_client_read(client->data, &len);
	
	/* grab current time and store it as client last activity */
	client->last_active = time(NULL);
	
	return len;
}

/* Sends data from a buffer to client. Returns number of sent bytes. */
int client_write(struct client_t *client, char *databuf, int datalen) {
	
	int len;
	
	/* handle special telnet characters to display them correctly on client */
	//telnet_handle_client_write(databuf, &datalen);
	
	//TODO let's print received bytes during development phase...
	if (debug_messages) {
		int i;
		for(i = 0; i < datalen; i++) {
			fprintf(stderr, "client %s -> %u '%c'\n",
					client->ip_string,
					(unsigned char) databuf[i],
					(unsigned char) databuf[i]);
		}	
	}
	
	/* send data to client */
	len = send(client->socket, databuf, datalen, 0);
	if (len == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	
	return len;
}

/* Waits for client to provide a username. Blocks until a username is entered. */
int client_ask_username(struct client_t *client) {
	
	int i;
	char databuf[DATABUF_LEN];
	fd_set read_fds;
	
	/* send username request to client */
	snprintf(databuf, DATABUF_LEN,
			 "\nPlease provide a username to identify yourself to other users (max %d characters):\n", USERNAME_LEN);
	client_write(client, databuf, strlen(databuf));
	
	/* wait for client input */
	client->username[0] = '\0';
	while (client->username[0] == '\0') {
		/* send prompt character to client */
		snprintf(databuf, DATABUF_LEN, "> ");
		client_write(client, databuf, strlen(databuf));
		/* setup select() parameters */
		FD_ZERO(&read_fds);
		FD_SET(client->socket, &read_fds);
		select((client->socket)+1, &read_fds, NULL, NULL, NULL);
		if (FD_ISSET(client->socket, &read_fds)) {
			client_read(client);
			/* handle client input */
			for (i = 0; i < USERNAME_LEN; i++) {
				if (client->data[i] == '\r') {
					break;
				}
				client->username[i] = client->data[i];
			}
			client->username[i+1] = '\0';
		}
	}
	
	/* send welcome message to client */
	snprintf(databuf, DATABUF_LEN,
			 "\nWelcome %s!\n\n", client->username);
	client_write(client, databuf, strlen(databuf));
	
	return 0;
}
