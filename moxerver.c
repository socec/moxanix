#include "moxerver.h"
#include <unistd.h> /* getopt() */


#define SERVER_WAIT_TIMEOUT 5 /* seconds for select() timeout in server loop */

/* Prints help message. */
void usage() {
	//TODO maybe some styling should be done
	fprintf(stderr, "Usage: moxerver -p tcp_port -t tty_path [-h]\n");
	fprintf(stderr, "- tcp_port must be in range 4001 to 4008\n\n");
}

/* MoxaNix main program loop. */
int main(int argc, char *argv[]) {
	
	int ret;
	
	struct server_t server;
	struct client_t client; //TODO working with only 1 client, this can be expanded into a list
	struct tty_t tty_dev;
	
	unsigned int tcp_port = -1;
	char tty_path[DEV_PATH] = {'\0'};
	
	fd_set read_fds;
	int fdmax;
	struct timeval tv;
	
	
	/* grab arguments */
	if (argc <= 1) {
		fprintf(stderr, "error parsing arguments\n");
		usage();
		return 0;
	}
	while ((ret = getopt(argc, argv, ":p:t:h")) != -1) {
		switch (ret) {
			/* get server port number */
			case 'p':
				tcp_port = (unsigned int) atoi(optarg);
				/* check port range */
				if (tcp_port < 4001 || tcp_port > 4008) {
					fprintf(stderr, "error: port number out of 4001-4008 range\n");
					return -1;
				}
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
				fprintf(stderr, "error parsing arguments\n");
				usage();
				return 0;
		}
	}
	if (strlen(tty_path) == 0) {
		fprintf(stderr, "error parsing arguments\n");
		usage();
		return 0;
	}
	
	/* introduction message */
	fprintf(stderr, "=== MoxaNix ===\n");
	
	//TODO remove the following line after development phase
	fprintf(stderr, "TCP port: %d, TTY device path: %s\n", tcp_port, tty_path); 
	
	/* initialize */
	server_setup(&server, tcp_port);
	client.socket = -1;
	
	
	//TODO this is a good place to create and start the TTY thread, use "tty_path" when opening device
	
	
	/* loop with timeouts waiting for client connection */
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
			//TODO do we really break and stop server when select returns an error?
			fprintf(stderr, "[%s:%d] error %d: %s\n", __func__, __LINE__, errno, strerror(errno));
			break;
		}
		if (ret > 0) {
			/* check server status */
			if (FD_ISSET(server.socket, &read_fds)) {
				fprintf(stderr, "received client connection request\n");
				/* accept connection request if there is no client connected yet */
				if (client.socket == -1) {
					ret = server_accept(&server, &client);
					if ( ret != 0) {
						/* print error but continue waiting for connection request */
						fprintf(stderr, "problem accepting client\n");
						continue;
					}
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
				if ( ret != 0) {
					/* print error but continue waiting for new data */
					fprintf(stderr, "problem reading client\n");
					continue;
				}
				//TODO we should send this data to TTY device
			}
		}
		if (ret == 0) {
			fprintf(stderr, "server waiting\n");
		}
		
	} /* END while loop */
	
	/* close server and client */
	server_close(&server);
	client_close(&client);
	
	return 0;
}