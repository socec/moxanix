#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> /* TCP_NODELAY */
#include <arpa/inet.h>
#include <termios.h>
#include <time.h>

#define DATABUF_LEN 128
#define DEV_PATH 128
#define TIMESTAMP_FORMAT "%Y-%m-%dT%H:%M:%S" /* ISO 8601 */
#define TIMESTAMP_LEN 20+1 /* calculated following the timestamp format */
#define CONFILE "moxanix.cfg"
#define USERNAME_LEN 32

/* Structures used for communication parameters. */
typedef struct
{
	int socket;						/* server socket */
	struct sockaddr_in address;		/* server address information */
	unsigned int port;				/* server port in host byte order, practical reference */
} server_t;

typedef struct
{
	int socket;							/* client socket */
	struct sockaddr_in address;			/* client address information */
	char ip_string[INET_ADDRSTRLEN];	/* client IP address as a string */
	time_t last_active;					/* time of client's last activity in seconds from Epoch */
	char username[USERNAME_LEN];		/* username for human identification */
	char data[DATABUF_LEN];				/* buffer for data received from client */
} client_t;

typedef struct
{
	int fd;						/* tty file descriptor */
	struct termios ttysetdef;	/* default tty termios settings */
	struct termios ttyset;		/* tty termios settings */
	char path[DEV_PATH];		/* tty device path */
	char data[DATABUF_LEN];		/* buffer for data received from tty */
} tty_t;


/* Global variables used throughout the application. */
int debug_messages;		/* if > 0 debug messages will be printed */
server_t server;		/* main server structure */
client_t client;		/* connected client structure */ //TODO working with only 1 client, this can be expanded into a list
client_t new_client;	/* client structure for new client request */
tty_t tty_dev;			/* connected tty device */


/* Global functions used throughout the application. */

/**
 * Converts from time in seconds from Epoch to conveniently formatted string.
 * 
 * Returns:
 * 0 always
 */
int time2string(time_t time, char* timestamp);


/* Functions handling server operation. */

/**
 * Sets up the server on specific port, binds to a socket and listens for client connections.
 *
 * Returns:
 * - 0 on success,
 * - negative errno value set appropriately by error in setup process
 */
int server_setup(server_t *server, unsigned int port);

/**
 * Closes the server socket.
 * 
 * Returns:
 * 0 always, but internally tries closing again if it fails
 */
int server_close(server_t *server);

/**
 * Accepts incoming client connection.
 * 
 * Returns:
 * - 0 on success,
 * - negative errno value set appropriately by error in setup process
 */
int server_accept(server_t *server, client_t *accepted_client);

/**
 * Thread function handling new client connections.
 * If there is no connected client then first client request is accepted.
 * If there is a connected client then new client is asked if connected client should be dropped.
 * 
 * Returns:
 * Return value from this thread function is not used.
 * Function handles global client variables:
 * - client structure is reset if new client is available to connect to
 * - new_client structure stores information about the client to connect to
 */
void* server_new_client_thread(void *args);


/* Functions handling communication with clients. */

/**
 * Closes client connection.
 * 
 * Returns:
 * 0 always, but internally tries closing again if it fails
 */
int client_close(client_t *client);

/**
 * Reads data from client into client data buffer.
 * Also updates client's last activity timestamp.
 * 
 * Returns:
 * - number of read bytes on success,
 * - negative ENODATA value (-ENODATA) if client disconnected,
 * - negative errno value set appropriately by error in reading
 */
int client_read(client_t *client);

/**
 * Sends data from a buffer to client.
 * 
 * Returns:
 * - number of sent bytes on success,
 * - negative errno value set appropriately by error in sending
 */
int client_write(client_t *client, char *databuf, int datalen);

/**
 * Waits for client input in "line mode", where client sends a whole line of characters.
 * Blocks until input arrives.
 * 
 * Returns:
 * - 0 on success
 * - negative value if error occurred
 */
int client_wait_line(client_t *client);

/**
 * Waits for client to provide a username.
 * Blocks until a username is entered.
 *
 * Returns:
 * - 0 on success
 * - negative value if error occurred
 */
int client_ask_username(client_t *client);


/* Functions handling details related to telnet protocol. */

/**
 * Tells client to go into "character" mode.
 * 
 * Returns:
 * - 0 on success
 * - negative value if error occurred
 */
int telnet_set_character_mode(client_t *client);

/**
 * Handles special characters in data buffer after receiving them from client.
 * Used to filter out handshake commands of telnet protocol.
 * 
 * Returns:
 * 0 always
 */
int telnet_handle_client_read(char *databuf, int *datalen);

/**
 * Handles special characters in data buffer before sending to client.
 * Used for echoing characters correctly to telnet client.
 * 
 * Returns:
 * 0 always
 */
int telnet_handle_client_write(char *databuf, int *datalen);


/* Functions handling communication with tty device. */

/* Opens the tty device and configures it. */
int tty_open(tty_t *tty_dev);

/* Closes the tty device. */
int tty_close(tty_t *tty_dev);

/* Reconfigures the tty device. */
int tty_reconfigure(tty_t *tty_dev, struct termios newttyset);

/* Reads incoming data from tty device to tty data buffer. */
int tty_read(tty_t *tty_dev);

/* Sends data from a buffer to tty device. */
int tty_write(tty_t *tty_dev, char *databuf, int datalen);

/* Main tty thread function */
void *tty_thread_func(void *arg);
int speed_to_baud(speed_t speed);
speed_t baud_to_speed(int baud);

