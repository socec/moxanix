/* Handles server operation. */

#pragma once

#include <common.h>
#include <client.h>

/* The serial-server scenario makes sense only for 1 connected client.
 * Allow 1 extra connection to reject new clients with an explanation. */
#define SERVER_MAX_CONNECTIONS 2

typedef struct
{
	int socket;					/* server socket */
	struct sockaddr_in address;	/* server address information */
	unsigned int port;			/* server port in host byte order */
} server_t;

/**
 * Sets up the server on a specific port, binds to a socket and listens for
 * client connections.
 *
 * Returns:
 * - 0 on success,
 * - negative errno value set by an error in the setup process
 */
int server_setup(server_t *server, unsigned int port);

/**
 * Closes the server socket.
 */
void server_close(server_t *server);

/**
 * Accepts an incoming client connection.
 *
 * Returns:
 * - 0 on success,
 * - negative errno value set by an error in the process
 */
int server_accept(server_t *server, client_t *accepted_client);
