/* 
 * Main server application.
 * Handles client connections on specific TCP port and allows bidirectional
 * communication with a specific TTY device.
 */

#include "moxerver.h"
#include <signal.h> /* handling quit signals */
#include <pthread.h>

#define NAME "moxerver"

#define SERVER_WAIT_TIMEOUT 5 /* seconds for select() timeout in server loop */

/* Prints help message. */
static void usage() {
	//TODO maybe some styling should be done
	fprintf(stderr, "Usage: %s -p tcp_port -t tty_path [-d] [-h]\n", NAME);
	fprintf(stderr, "\t-d\tturns on debug messages\n");
	fprintf(stderr, "\n");
}

/* Performs cleanup and exit. */
void cleanup(int exit_code) {
	fprintf(stderr, "[%s] cleanup and exit with %d\n", NAME, exit_code);
	/* close client */
	if (client.socket != -1) {
		client_close(&client);
	}
	/* close tty device */
	tty_close(&tty_dev);
	/* close server */
	server_close(&server);
	exit(exit_code);
}

/* Handles received quit signals, use it for all quit signals of interest. */
void quit_handler(int signum) {
    /* perform cleanup and exit with 0 */
	fprintf(stderr, "[%s] received signal %d\n", NAME, signum);
	cleanup(0);
}

/* Converts from time in seconds from Epoch to conveniently formatted string. */
int time2string(time_t time, char* timestamp) {
	strftime(timestamp, TIMESTAMP_LEN, TIMESTAMP_FORMAT, localtime(&time));
	return 0;
}

/* MoxaNix main program loop. */
int main(int argc, char *argv[]) {
	
	int ret;
	
	unsigned int tcp_port = -1;
	
	fd_set read_fds;
	int fdmax;
	struct timeval tv;

	pthread_t tty_thread; 
	
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
	debug_messages = 0;
	while ((ret = getopt(argc, argv, ":p:t:dh")) != -1) {
		switch (ret) {
			/* get server port number */
			case 'p':
				tcp_port = (unsigned int) atoi(optarg);
				break;
			/* get tty device path */
			case 't':
				if ((strnlen(optarg, DEV_PATH) == 0) || 
					(strnlen(optarg, DEV_PATH) > (DEV_PATH - 1))) {
					fprintf(stderr, "[%s] error: tty path was not specified\n\n", NAME);
					usage();
					return -1;
				} else {
					/* set tty device path in tty_dev struct */
					strcpy(tty_dev.path, optarg);
				}
				break;
			/* enable debug messages */
			case 'd':
				debug_messages = 1;
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
	
	/* introduction message */
	fprintf(stderr, "[%s] === MoxaNix ===\n", NAME);
	
	/* initialize */
	server_setup(&server, tcp_port);
	client.socket = -1;
	
	/* open tty device */
	if (tty_open(&tty_dev) < 0) {
		fprintf(stderr, "[%s] error: opening of tty device at %s failed\n"
				"\t\t-> continuing in echo mode\n", NAME, tty_dev.path); 
		//return -1;
	}
	
	/* start thread that handles tty device */
	ret = pthread_create(&tty_thread, NULL, tty_thread_func, &tty_dev); //TODO check return value?
	
	/* loop with timeouts waiting for client connection and data */
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
				/* accept connection request if no client is connected */
				if (client.socket == -1) {
					ret = server_accept(&server, &client);
					if ( ret != 0) {
						/* print error but continue waiting for connection request */
						//TODO maybe we should break here to avoid endless loop, what are possible causes of this failure?
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
				/* check if client disconnected or other errors occurred */
				if (ret == -ENODATA) {
					fprintf(stderr, "[%s] client %s disconnected\n", NAME, client.ip_string);
					/* close client connection and continue waiting for new clients */
					client_close(&client);
					continue;
				}
				if ( ret < 0) {
					/* print error but continue waiting for new data */
					fprintf(stderr, "[%s] problem reading client\n", NAME);
					continue;
				}
				/* pass received data to tty device */
				tty_write(&tty_dev, client.data, ret); 
			}
		}
		if (ret == 0) {
			/* do something with inactive client */
			if (client.socket != -1) {
				//TODO we could drop client if inactive for some time
				time_t current_time = time(NULL);
				if (debug_messages) {
					fprintf(stderr, "[%s] client last active %u seconds ago\n", NAME,
							(unsigned int) (current_time - client.last_active));
				}
			}
			/* do something while listening for client connections */
			else {
				if (debug_messages) {
					fprintf(stderr, "[%s] listening for client connection\n", NAME);
				}
			}
		}
		
	} /* END while() loop */
	
	/* unexpected break from while() loop */
	fprintf(stderr, "[%s] unexpected condition\n", NAME);
	/* cleanup and exit with -1 */
	cleanup(-1);
	
	return -1;
}
