/* 
 * Handling communication with clients.
 */

#include "moxerver.h"

/* Closes client connection. */
int client_close(client_t *client) {
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
int client_read(client_t *client) {
	
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
int client_write(client_t *client, char *databuf, int datalen) {
	
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

/* Waits for client input in "line mode". Blocks until input arrives. */
int client_wait_line(client_t *client) {
	
	fd_set read_fds;
	struct timeval tv;
	
	client->data[0] = '\0';
	/* loop waiting for client input */
	while (client->data[0] == '\0') {
		/* setup select() parameters */
		tv.tv_sec = 15; /* 15 second timeout */
		tv.tv_usec = 0;
		FD_ZERO(&read_fds);
		FD_SET(client->socket, &read_fds);
		/* send prompt character to client */
		client_write(client, "> ", 2);
		/* block until input arrives */
		if (select((client->socket)+1, &read_fds, NULL, NULL, &tv) <= 0) return -1;
		if (FD_ISSET(client->socket, &read_fds)) {
			/* read client input */
			if (client_read(client) == -1) return -1;
			/* we don't want empty data so stop on \r or \n */
			if ( (client->data[0] == '\r') || (client->data[0] == '\n') ) {
				client->data[0] = '\0';
			}
		}
	}
	
	return 0;
}

/* Waits for client to provide a username. Blocks until a username is entered. */
int client_ask_username(client_t *client) {
	
	int i;
	char msg[DATABUF_LEN];
	
	/* send username request to client */
	snprintf(msg, DATABUF_LEN,
			 "\nPlease provide a username to identify yourself to other users (max %d characters):\n", USERNAME_LEN);
	client_write(client, msg, strlen(msg));
	
	/* wait for client input */
	if (client_wait_line(client) != 0) return -1;
	
	/* save received data as client username */
	for (i = 0; i < USERNAME_LEN; i++) {
		if ( (client->data[i] == '\r') || (client->data[i] == '\n') ) {
			/* don't include \r or \n in username */
			client->username[i] = '\0';
			break;
		}
		client->username[i] = client->data[i];
	}
	
	/* send welcome message to client */
	snprintf(msg, DATABUF_LEN,
			 "\nWelcome %s!\n\n", client->username);
	client_write(client, msg, strlen(msg));
	
	return 0;
}
