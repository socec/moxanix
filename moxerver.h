#pragma once

#include <common.h>
#include <telnet.h>
#include <client.h>
#include <server.h>
#include <tty.h>

#define CONFILE "moxanix.cfg"

#define SERVER_WAIT_TIMEOUT 2 /* seconds for select() timeout in server loop */
#define TTY_WAIT_TIMEOUT 5 /* seconds for select() timeout in tty loop */

/* Global variables used throughout the application. */
server_t server;	 /* main server */
client_t client;	 /* connected client */ //TODO working with only 1 client, this can be expanded into a list
client_t new_client; /* reserved for a new client request */
tty_t tty_dev;		 /* connected tty device */

typedef struct
{
	server_t *server;
	client_t *client;
	client_t *new_client;
	tty_t *tty_dev;
} resources_t;

/**
 * The thread function handling new client connections.
 *
 * If there is no connected clients then the first client request is accepted.
 * If there is a connected client then the new client is asked if the currently
 * connected client should be dropped.
 *
 * The function handles global resources through the pointer to a "resources_t"
 * structure passed as the input argument.
 *
 * Returns:
 * Return value from this thread function is not used.
 */
void* thread_new_client_connection(void *args);

/**
 * The thread function handling data from the tty device.
 *
 * The function handles global resources through the pointer to a "resources_t"
 * structure passed as the input argument.
 *
 * Returns:
 * Return value from this thread function is not used.
 */
void* thread_tty_data(void *args);

/**
 * The thread function handling data from the connected client.
 *
 * The function handles global resources through the pointer to a "resources_t"
 * structure passed as the input argument.
 *
 * Returns:
 * Return value from this thread function is not used.
 */
void* thread_client_data(void *args);
