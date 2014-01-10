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
#define DEV_PATH 32
#define TIMESTAMP_FORMAT "%d.%m.%Y. %H:%M:%S"
#define TIMESTAMP_LEN 20+1 /* calculated following timestamp format */
#define CONFILE "moxanix.cfg"

/* Structures used for communication parameters. */
struct server_t {
	int socket; 					/* server socket */
	struct sockaddr_in address; 	/* server address information */
	unsigned int port;				/* server port in host byte order, practical reference */
};

struct client_t {
	int socket; 						/* client socket */
	struct sockaddr_in address; 		/* client address information */
	char ip_string[INET_ADDRSTRLEN]; 	/* client IP address as a string */
	time_t last_active; 				/* time of client's last activity in seconds from Epoch */
	char data[DATABUF_LEN]; 			/* buffer for data received from client */
};

struct tty_t {
	int fd; 					/* tty file descriptor */
	struct termios ttysetdef;	/* default tty termios settings */
	struct termios ttyset;		/* tty termios settings */
	char path[DEV_PATH]; 		/* tty device path */
	char data[DATABUF_LEN]; 	/* buffer for data received from tty */
};


/* Global variables used throughout the application. */
int debug_messages; 		/* if > 0 debug messages will be printed */
struct server_t server; 	/* main server structure */
struct client_t client; 	/* connected client structure */ //TODO working with only 1 client, this can be expanded into a list
struct tty_t tty_dev;		/* connected tty device */


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
int server_setup(struct server_t *server, unsigned int port);

/**
 * Closes the server socket.
 * 
 * Returns:
 * 0 always, but internally tries closing again if it fails
 */
int server_close(struct server_t *server);

/**
 * Accepts incoming client connection.
 * 
 * Returns:
 * - 0 on success,
 * - negative errno value set appropriately by error in setup process
 */
int server_accept(struct server_t *server, struct client_t *accepted_client);

/**
 * Rejects incoming client connection.
 * 
 * Returns:
 * 0 always, errors with rejected client are ignored
 */
int server_reject(struct server_t *server);


/* Functions handling communication with clients. */

/**
 * Closes client connection.
 * 
 * Returns:
 * 0 always, but internally tries closing again if it fails
 */
int client_close(struct client_t *client);

/**
 * Reads data from client into client data buffer.
 * Also updates client's last activity timestamp.
 * 
 * Returns:
 * - number of read bytes on success,
 * - negative ENODATA value (-ENODATA) if client disconnected,
 * - negative errno value set appropriately by error in reading
 */
int client_read(struct client_t *client);

/**
 * Sends data from a buffer to client.
 * 
 * Returns:
 * - number of sent bytes on success,
 * - negative errno value set appropriately by error in sending
 */
int client_write(struct client_t *client, char *databuf, int datalen);


/* Functions handling details related to telnet protocol. */

/**
 * Tells client to go into "character" mode.
 * 
 * Returns:
 * - 0 on success
 * - negative value if error occurred
 */
int telnet_set_character_mode(struct client_t *client);

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
int tty_open(struct tty_t *tty_dev);

/* Closes the tty device. */
int tty_close(struct tty_t *tty_dev);

/* Reconfigures the tty device. */
int tty_reconfigure(struct tty_t *tty_dev, struct termios newttyset);

/* Reads incoming data from tty device to tty data buffer. */
int tty_read(struct tty_t *tty_dev);

/* Sends data from a buffer to tty device. */
int tty_write(struct tty_t *tty_dev, char *databuf, int datalen);

/* Main tty thread function */
void *tty_thread_func(void *arg);
int speed_to_baud(speed_t speed);
speed_t baud_to_speed(int baud);

