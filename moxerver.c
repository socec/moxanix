#include "moxerver.h"
#include <signal.h> /* handling quit signals */

#define NAME "moxerver"

#define SERVER_WAIT_TIMEOUT 5 /* seconds for select() timeout in server loop */
#define PORT_MIN 4001 /* minimum TCP port number */
#define PORT_MAX 4008 /* maximum TCP port number */


/* Global variables used throughout the application */
struct server_t server;
struct client_t client; //TODO working with only 1 client, this can be expanded into a list
struct tty_t tty_dev;

/* Prints help message. */
static void usage() {
	//TODO maybe some styling should be done
	fprintf(stderr, "Usage: moxerver -p tcp_port -t tty_path [-h]\n");
	fprintf(stderr, "- tcp_port range [%d .. %d]\n\n", PORT_MIN, PORT_MAX);
}

/* Performs cleanup and exit. */
void cleanup(int exit_code) {
	fprintf(stderr, "[%s] cleanup and exit with %d\n", NAME, exit_code);
	/* close server and client */
	server_close(&server);
	if (client.socket != -1) {
		client_close(&client);
	}
	exit(exit_code);
}

/* Handles received quit signals, use it for all quit signals of interest. */
void quit_handler(int signum) {
    /* perform cleanup and exit with 0 */
	fprintf(stderr, "[%s] received signal %d\n", NAME, signum);
	cleanup(0);
}

/* MoxaNix main program loop. */
int main(int argc, char *argv[]) {
	
	int ret;
	
	unsigned int tcp_port = -1;
	char tty_path[DEV_PATH] = {'\0'};
	
	fd_set read_fds;
	int fdmax;
	struct timeval tv;
	
	
	/* catch and handle some quit signals, SIGKILL can't be caught */
	signal(SIGTERM, quit_handler);
	signal(SIGQUIT, quit_handler);
	signal(SIGINT, quit_handler);
	
	/* check argument count */
	if (argc <= 1) {
		usage();
		return -1;
	}
	/* grab arguments */
	while ((ret = getopt(argc, argv, ":p:t:h")) != -1) {
		switch (ret) {
			/* get server port number */
			case 'p':
				tcp_port = (unsigned int) atoi(optarg);
				break;
			/* get tty device path */
			case 't':
				sprintf(tty_path, optarg);
				break;
			/* print help and exit */
			case 'h':
				usage();
				return 0;
			default:
				fprintf(stderr, "[%s] error parsing arguments\n", NAME);
				usage();
				return -1;
		}
	}
	/* check arguments */
	if (tcp_port < PORT_MIN || tcp_port > PORT_MAX) {
		fprintf(stderr, "[%s] error: port number out of %d-%d range\n\n",
				NAME, PORT_MIN, PORT_MAX);
		usage();
		return -1;
	}
	if (strlen(tty_path) == 0) {
		fprintf(stderr, "[%s] error: tty path was not specified\n\n", NAME);
		usage();
		return -1;
	}
	
	/* introduction message */
	fprintf(stderr, "[%s] === MoxaNix ===\n", NAME);
	
	//TODO remove the following line after development phase
	fprintf(stderr, "[%s] TCP port: %d, TTY device path: %s\n", NAME, tcp_port, tty_path); 
	
	/* initialize */
	server_setup(&server, tcp_port);
	client.socket = -1;
	
	
	//TODO this is a good place to create and start the TTY thread, use "tty_path" when opening device
	
	
	/* loop with timeouts waiting for client connection and data*/
	while (1) {
		
		/* setup parameters for select() */
		tv.tv_sec = SERVER_WAIT_TIMEOUT;
		tv.tv_usec = 0;
		FD_ZERO(&read_fds);
		FD_SET(server.socket, &read_fds);
		if (client.socket != -1) {
			FD_SET(client.socket, &read_fds); /* wait for client if connected */
		}
		fdmax = (server.socket > client.socket) ? server.socket : client.socket;		
		
		/* wait with select() */
		ret = select(fdmax+1, &read_fds, NULL, NULL, &tv);
		if (ret == -1) {
			//TODO do we really break here and stop server when select returns an error?
			fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
			break;
		}
		if (ret > 0) {
			/* check server status */
			if (FD_ISSET(server.socket, &read_fds)) {
				fprintf(stderr, "[%s] received client connection request\n", NAME);
				/* accept connection request if there is no client connected yet */
				if (client.socket == -1) {
					ret = server_accept(&server, &client);
					if ( ret != 0) {
						/* print error but continue waiting for connection request */
						fprintf(stderr, "[%s] problem accepting client\n", NAME);
						continue;
					}
					/* put client in "character" mode */
					telnet_set_character_mode(&client);
				}
				/* reject connection request if a client is already connected */
				else {
					server_reject(&server);
				}
			}
			/* check client status if connected */
			if ( (client.socket != -1) && FD_ISSET(client.socket, &read_fds) ) {
				/* read client data */
				ret = client_read(&client);
				if ( ret < 0) {
					/* print error but continue waiting for new data */
					fprintf(stderr, "[%s] problem reading client\n", NAME);
					continue;
				}
				/* echo back to client */
				client_write(&client, client.data, ret);
				
				//TODO we should send this data to TTY device here
			}
		}
		if (ret == 0) {
			/* check if client disconnected */
			/* a disconnected client socket is ready for reading but read returns 0 */
			if ( (client.socket != -1) && FD_ISSET(client.socket, &read_fds) ) {
				if (client_read(&client) == 0) {
					fprintf(stderr, "[%s] client %s disconnected\n", NAME, client.ip_string);
					/* close client connection */
					client_close(&client);
				}
			}
			else {
				fprintf(stderr, "[%s] server waiting\n", NAME);
			}
		}
		
	} /* END while() loop */
	
	/* unexpected break from while() loop */
	fprintf(stderr, "[%s] unexpected condition\n", NAME);
	/* cleanup and exit with -1 */
	cleanup(-1);
	
	return -1;
}