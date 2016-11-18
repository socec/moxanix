/* Handles communication with clients. */

#include <moxerver.h>

/**
 * Closes a client connection.
 */
void client_close(client_t *client);

/**
 * Reads data from a client into the client data buffer.
 * Also updates the timestamp of the client's last activity.
 *
 * Returns:
 * - number of read bytes on success,
 * - negative ENODATA value (-ENODATA) if the client has disconnected,
 * - negative errno value set by an error while reading
 */
int client_read(client_t *client);

/**
 * Sends data from a buffer to the client.
 *
 * Returns:
 * - number of sent bytes on success,
 * - negative errno value set by an error while sending
 */
int client_write(client_t *client, char *databuf, int datalen);

/**
 * Waits for input from the client in "line mode" (client sends a whole line of
 * characters). The function blocks until input arrives.
 *
 * Returns:
 * - 0 on success
 * - negative value if error occurred
 */
int client_wait_line(client_t *client);

/**
 * Asks the client to provide a username.
 * Blocks until a string is provided.
 *
 * Returns:
 * - 0 on success
 * - negative value if error occurred
 */
int client_ask_username(client_t *client);
