#include "moxerver_include.h"


/* Sets up the server on specific port, binds to a socket and listens for client connections. */
int server_setup(struct server_t *server, unsigned int port) {
	
	int namelen, opt;
	
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
	fprintf(stderr,"[%s]: socket created\n", __func__);
	
	/* try to avoid "Address already in use" error */
	opt = 1; /* true value for setsockopt option */
	if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	
	/* bind server address to a socket */
	if (bind(server->socket, (struct sockaddr *) &server->address, sizeof(server->address)) == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	fprintf(stderr,"[%s]: bind successful\n", __func__);
	
	/* check port assignment */
	namelen = sizeof(server->address);
	if (getsockname(server->socket, (struct sockaddr *) &server->address, &namelen) == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	if (ntohs(server->address.sin_port) != port) {
		fprintf(stderr, "[%s:%d] error: could not assign port %u\n", __func__, __LINE__, port);
		return -1;
	}
	/* save server port number */
	server->port = port;
	fprintf(stderr,"[%s]: assigned port %u\n", __func__, server->port); //ntohs(server->address.sin_port)
	
	/* listen for a client connection, allow 2 connections in queue */
	if (listen(server->socket, 2) == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	
	fprintf(stderr,"[%s]: server is up, listening for client connection\n", __func__);
	return 0;
}

/* Closes the server. */
int server_close(struct server_t *server) {
	/* force closing in case of error */
	if (close(server->socket) == -1) {
		close(server->socket);
	}
	fprintf(stderr,"[%s]: socket closed, server is down\n", __func__);
	return 0;
}

/* Accepts incoming client connection. */
int server_accept(struct server_t *server, struct client_t *accepted_client) {
	
	int namelen;
	
	/* accept connection request */
	namelen = sizeof(accepted_client->address);
	accepted_client->socket = accept(server->socket, (struct sockaddr *) &accepted_client->address, &namelen);
	if (accepted_client->socket == -1) {
		fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
		return -errno;
	}
	
	/* get client IP address as human readable string */
	inet_ntop(accepted_client->address.sin_family, &accepted_client->address.sin_addr.s_addr, 
				accepted_client->ip_string, INET_ADDRSTRLEN);
	
	/* print client information */
	//TODO also print timestamp
	fprintf(stderr, "[%s]: accepted client %s on port %u\n", __func__,
			accepted_client->ip_string, server->port);
	return 0;
}

/* Rejects incoming client connection. */
int server_reject(struct server_t *server) {
	
	int namelen;
	struct client_t rclient;
	char reject_msg[128];
	
	/* accept connection request */
	namelen = sizeof(rclient.address);
	rclient.socket = accept(server->socket, (struct sockaddr *) &rclient.address, &namelen);
	/* send reject message */
	sprintf(reject_msg, "[%s]: port %u is already being used\n", __func__, server->port);
	send(rclient.socket, reject_msg, strlen(reject_msg), 0);
	/* close connection */
	close(rclient.socket);
	
	fprintf(stderr, "[%s]: rejected new client request, there is alredy a client connected\n", __func__, server->port);
	return 0;
}
