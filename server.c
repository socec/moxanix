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
	
	/* listen for a client connection, allow 2 connections in queue */
	if (listen(server->socket, 2) == -1) {
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
	fprintf(stderr, "[%s] accepted client %s on port %u @ %s\n", __func__,
			accepted_client->ip_string, server->port, timestamp);
	return 0;
}

/* Drops current client or rejects new client. */
int server_drop(struct server_t *server, struct client_t *current_client) {
	
	struct client_t new_client;
	char msg[DATABUF_LEN];
	char timestamp[TIMESTAMP_LEN];
	
	/* accept new connection request */
	if (server_accept(server, &new_client) != 0) return -1;
	
	/* inform new client that port is already in use */
	time2string(current_client->last_active, timestamp);
	sprintf(msg, "\nPort %u is already being used!\nCurrent user and last activity:\n%s @ %s\n",
			server->port, current_client->username, timestamp);
	send(new_client.socket, msg, strlen(msg), 0);
	
	/* ask new client if current client should be dropped */
	sprintf(msg, "\nDo you want to drop the current user?\nIf yes then please type YES DROP (in uppercase):");
	send(new_client.socket, msg, strlen(msg), 0);
	
	/* wait for client input */
	if (client_wait_line(&new_client) != 0) return -1;
	
	/* check client confirmation */
	if (strncmp(new_client.data, "YES DROP", 8) == 0) {
		/* current client should be dropped, drop it */
		client_close(current_client);
		time2string(time(NULL), timestamp);
		fprintf(stderr, "[%s] dropped current client %s on port %u @ %s\n", __func__,
				current_client->ip_string, server->port, timestamp);
		/* make new client the current client */
		memcpy(current_client, &new_client, sizeof(struct client_t));
		time2string(time(NULL), timestamp);
		fprintf(stderr, "[%s] accepted new client request %s on port %u @ %s\n", __func__,
				current_client->ip_string, server->port, timestamp);
		/* return 1 because current client was dropped */
		return 1;
	}
	else {
		/* new connection should be rejected, close connection */
		close(new_client.socket);
		time2string(time(NULL), timestamp);
		fprintf(stderr, "[%s] rejected new client request %s on port %u @ %s\n", __func__,
				new_client.ip_string, server->port, timestamp);
		/* return 0 because new client was rejected */
		return 0;
	}
	
	
}
