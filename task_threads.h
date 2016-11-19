/* Thread functions for handling top level tasks. */

#pragma once

#include <client.h>
#include <server.h>
#include <tty.h>
#include <pthread.h>

#define SERVER_WAIT_TIMEOUT 2 /* seconds for select() timeout in server loop */
#define TTY_WAIT_TIMEOUT 5 /* seconds for select() timeout in tty loop */

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
 * If there is no connected client then the first client request is accepted.
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
 * The incoming tty device data is sent directly to the connected client.
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
 * The incoming client data is sent directly to the tty device.
 *
 * The function handles global resources through the pointer to a "resources_t"
 * structure passed as the input argument.
 *
 * Returns:
 * Return value from this thread function is not used.
 */
void* thread_client_data(void *args);
