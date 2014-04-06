/* 
 * Handling server operation.
 */

#include "moxerver.h"

/* Sets up the server on specific port, binds to a socket and listens for client connections. */
int server_setup(struct server_t *server, unsigned int port) {
	
	int opt;
	char timestamp[TIMESTAMP_LEN];
	
	/* set up server address information */
	server->address.sin_family = AF_INET; 			/* use IPv4 address family */
	server->address.sin_port = htons(port); 		/* set up port number, htons is for using network byte order */
	server->address.sin_addr.s_addr = INADDR_ANY; 	/* use local address */
	
	/* create stream socket using TCP */
	server->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server->socket == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	fprintf(stderr,"[%s] socket created\n", __func__);
	
	/* try to avoid "Address already in use" error */
	opt = 1; /* true value for setsockopt option */
	if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	/* turn off Nagle algorithm */
	opt = 1; /* true value for setsockopt option */
	if (setsockopt(server->socket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int)) == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	
	/* bind server address to a socket */
	if (bind(server->socket, (struct sockaddr *) &server->address, sizeof(server->address)) == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	fprintf(stderr,"[%s] bind successful\n", __func__);
	
	/* save server port number */
	server->port = port;
	fprintf(stderr,"[%s] assigned port %u\n", __func__, server->port); //ntohs(server->address.sin_port)
	
	/* listen for a client connection, allow some connections in queue */
	if (listen(server->socket, 1) == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	
	time2string(time(NULL), timestamp);
	fprintf(stderr,"[%s] server is up @ %s\n", __func__, timestamp);
	return 0;
}

/* Closes the server socket. */
int server_close(struct server_t *server) {
	char timestamp[TIMESTAMP_LEN];
	/* force closing in case of error */
	if (close(server->socket) == -1) {
		close(server->socket);
	}
	time2string(time(NULL), timestamp);
	fprintf(stderr,"[%s] socket closed, server is down @ %s\n", __func__, timestamp);
	return 0;
}

/* Accepts incoming client connection. */
int server_accept(struct server_t *server, struct client_t *accepted_client) {
	
	int namelen;
	char timestamp[TIMESTAMP_LEN];
	
	/* accept connection request */
	namelen = sizeof(accepted_client->address);
	accepted_client->socket = accept(server->socket, (struct sockaddr *) &accepted_client->address, (socklen_t *) &namelen);
	if (accepted_client->socket == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	/* make client socket non-blocking */
	if (fcntl(accepted_client->socket, F_SETFL, O_NONBLOCK) == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	
	/* get client IP address as human readable string */
	inet_ntop(accepted_client->address.sin_family, &accepted_client->address.sin_addr.s_addr, 
				accepted_client->ip_string, INET_ADDRSTRLEN);
	
	/* grab current time and store it as client last activity*/
	accepted_client->last_active = time(NULL);
	
	/* print client information */
	time2string(accepted_client->last_active, timestamp);
	fprintf(stderr, "[%s] accepted client %s @ %s\n",
			__func__, accepted_client->ip_string, timestamp);
	return 0;
}

/* Thread function handling new client connections. Handles global client variables. */
void* server_new_client_thread(void *args) {
	
	char msg[DATABUF_LEN];
	char timestamp[TIMESTAMP_LEN];
	struct client_t temp_client;
	
	/* accept new connection request */
	if (server_accept(&server, &temp_client) != 0) return (void *) -1;
	
	/* if no client is connected then make new client available for connection */
	if (client.socket == -1) {
		memcpy(&new_client, &temp_client, sizeof(struct client_t));
		return (void *) 0;
	}
	
	/* if there is already a new client being handled then reject this client request */
	if (new_client.socket != -1) {
		sprintf(msg, "\nToo many connection requests, please try later.\n");
		send(temp_client.socket, msg, strlen(msg), 0);
		client_close(&temp_client);
		time2string(time(NULL), timestamp);
		fprintf(stderr, "[%s] rejected new client request %s @ %s\n",
				__func__, temp_client.ip_string, timestamp);
		return (void *) 0;
	}
	
	/* make new client available for connection */
	memcpy(&new_client, &temp_client, sizeof(struct client_t));
	
	/* inform new client that port is already in use */
	time2string(client.last_active, timestamp);
	sprintf(msg, "\nPort %u is already being used!\nCurrent user and last activity:\n%s @ %s\n",
			server.port, client.username, timestamp);
	send(new_client.socket, msg, strlen(msg), 0);
	
	/* ask new client if current client should be dropped */
	sprintf(msg, "\nDo you want to drop the current user?\nIf yes then please type YES DROP (in uppercase):\n");
	send(new_client.socket, msg, strlen(msg), 0);
	
	/* wait for client input */
	client_wait_line(&new_client);
	
	/* check client confirmation */
	if (strncmp(new_client.data, "YES DROP", 8) == 0) {
		/* drop connected client */
		client_close(&client);
		fprintf(stderr, "[%s] dropped client %s @ %s\n",
			   __func__, client.ip_string, timestamp);
	}
	else {
		/* reject this client request */
		client_close(&new_client);
		time2string(time(NULL), timestamp);
		fprintf(stderr, "[%s] rejected new client request %s @ %s\n",
				__func__, temp_client.ip_string, timestamp);
		return (void *) 1;
	}
	
	return (void *) 0;
}
