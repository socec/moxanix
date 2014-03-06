#include "moxerver.h"


/* Closes client connection. */
int client_close(struct client_t *client) {
	/* force closing in case of error */
	if (close(client->socket) == -1) {
		close(client->socket);
	}
	client->socket = -1;
	fprintf(stderr,"[%s]: socket closed for client %s\n", __func__, client->ip_string);
	return 0;
}

/* Reads incoming data from client to client data buffer. */
int client_read(struct client_t *client) {
	
	int len;
	
	/* read client data */
	len = recv(client->socket, client->data, sizeof(client->data)-1, 0);
	if (len == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	/* null-terminate received data */
	client->data[len] = '\0';
	
	//TODO how does a client disconnect? For now let's wait for QUIT command...
	if (!strncmp(client->data, "QUIT", 4)) {
		client_close(client);
	}
	
	//TODO let's print the data during development phase...
	fprintf(stderr, "client %s says: %s", client->ip_string, client->data);
	
	return 0;
}

/* Sends data from a buffer to client. */
int client_write(struct client_t *client, char *databuf, int datalen) {
	/* send data to client */
	if (send(client->socket, databuf, datalen, 0) == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	return 0;
}
