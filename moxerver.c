/* 
 * Main server application.
 * Handles client connections on specific TCP port and allows bidirectional
 * communication with a specific TTY device.
 */

#include <moxerver.h>
#include <parser.h>
#include <signal.h> /* handling quit signals */
#include <pthread.h>

#define NAME "moxerver"

/* Prints help message. */
static void usage()
{
	//TODO maybe some styling should be done
	fprintf(stderr, "Usage: %s -p tcp_port [-t tty_path] [-d] [-h]\n", NAME);
	fprintf(stderr, "\t-t\ttty dev path (if not specified %s needs to be defined)\n", CONFILE);
	fprintf(stderr, "\t-d\tturns on debug messages\n");
	fprintf(stderr, "\n");
}

/* Performs cleanup and exit. */
void cleanup(int exit_code)
{
	// TODO: maybe pthread_kill() should be used for threads and cleanup there
	fprintf(stderr, "[%s] cleanup and exit with %d\n", NAME, exit_code);
	/* close the client */
	if (client.socket != -1)
	{
		client_close(&client);
	}
	/* close the tty device */
	if (tty_dev.fd != -1)
	{
		tty_close(&tty_dev);
	}
	/* close the server */
	server_close(&server);
	/* exit */
	exit(exit_code);
}

/* Handles received quit signals, use it for all quit signals of interest. */
void quit_handler(int signum)
{
    /* perform cleanup and exit with 0 */
	fprintf(stderr, "[%s] received signal %d\n", NAME, signum);
	cleanup(0);
}

/* Parse handler function, used to configure serial port */
int parse_handler(void *user, const char *section, const char *name, const char *value)
{
	//printf("[%s] section = %s, name = %s, value = %s\n", __func__, section, name, value);
	
	if (!strcmp(name, "speed") && (unsigned int)atoi(section) == server.port)
	{
		fprintf(stderr, "[%s] setting %s speed for port %s\n", __func__, value, section);
		
		if (cfsetispeed(&(tty_dev.ttyset), baud_to_speed(atoi(value))) < 0 || 
			cfsetospeed(&(tty_dev.ttyset), baud_to_speed(atoi(value))) < 0)
		{
			fprintf(stderr, "[%s] error configuring tty device speed\n", NAME);
			return -1;
   		}
   	}
	
	if (!strcmp(name, "dev") && (unsigned int)atoi(section) == server.port)
	{
		strcpy(tty_dev.path, value);
	}

	return 1;
}

void time2string(time_t time, char* timestamp)
{
	strftime(timestamp, TIMESTAMP_LEN, TIMESTAMP_FORMAT, localtime(&time));
}

void* thread_new_client_connection(void *args)
{
	char msg[BUFFER_LEN];
	char timestamp[TIMESTAMP_LEN];
	client_t temp_client;

	/* get resources from args */
	resources_t r = *((resources_t*) args);

	/* accept new connection request */
	if (server_accept(r.server, &temp_client) != 0)
	{
		return (void *) -1;
	}

	/* if no client is connected then make the new client available for connection */
	if (r.client->socket == -1)
	{
		memcpy(r.new_client, &temp_client, sizeof(client_t));
		return (void *) 0;
	}

	/* if there is already a new client being handled then reject this client request */
	if (r.new_client->socket != -1)
	{
		sprintf(msg, "\nToo many connection requests, please try later.\n");
		send(temp_client.socket, msg, strlen(msg), 0);
		client_close(&temp_client);
		time2string(time(NULL), timestamp);
		fprintf(stderr, "[%s] rejected new client request %s @ %s\n", __func__,
				temp_client.ip_string, timestamp);
		return (void *) 0;
	}

	/* make the new client available for connection */
	memcpy(r.new_client, &temp_client, sizeof(client_t));

	/* inform the new client that the port is already in use */
	time2string(r.client->last_active, timestamp);
	sprintf(msg, "\nPort %u is already being used!\nCurrent user and last activity:\n%s @ %s\n",
			r.server->port, r.client->username, timestamp);
	send(r.new_client->socket, msg, strlen(msg), 0);

	/* ask the new client if the currently connected client should be dropped */
	sprintf(msg, "\nDo you want to drop the current user?\n"
				"If yes then please type YES DROP (in uppercase):\n");
	send(r.new_client->socket, msg, strlen(msg), 0);

	/* wait for client input */
	client_wait_line(r.new_client);

	/* check client confirmation */
	if (strncmp(r.new_client->data, "YES DROP", 8) == 0)
	{
		/* drop the connected client */
		client_close(r.client);
		fprintf(stderr, "[%s] dropped client %s @ %s\n", __func__,
				r.client->ip_string, timestamp);
	}
	else
	{
		/* reject this client request */
		client_close(r.new_client);
		time2string(time(NULL), timestamp);
		fprintf(stderr, "[%s] rejected new client request %s @ %s\n", __func__,
				temp_client.ip_string, timestamp);
		return (void *) 1;
	}

	return (void *) 0;
}

void* thread_tty_data(void *args)
{
	struct timeval tv;
	ssize_t br = 0;
	int ret;
	fd_set read_fds;

	/* get resources from args */
	resources_t r = *((resources_t*) args);

	fprintf(stderr, "[%s] tty thread started with passed argument: %s\n",
				NAME, r.tty_dev->path);

	while (1)
	{
		/* setup parameters for select() */
		tv.tv_sec = TTY_WAIT_TIMEOUT;
		tv.tv_usec = 0;
		FD_ZERO(&read_fds);
		FD_SET(r.tty_dev->fd, &read_fds);

		/* wait with select() */
		ret = select(r.tty_dev->fd + 1, &read_fds, NULL, NULL, &tv);

		if (ret > 0 && FD_ISSET(r.tty_dev->fd, &read_fds))
		{
			br = tty_read(r.tty_dev);
			client_write(r.client, r.tty_dev->data, br);
		}
		else
		{

		}

		if (debug_messages)
		{
			fprintf(stderr, "[%s] tty thread alive\n", NAME);
		}
	}

	fprintf(stderr, "[%s] tty thread stoped\n", NAME);

	return (void*) 0;
}

void* thread_client_data(void *args)
{
	int ret;

	fd_set read_fds;
	int fdmax;
	struct timeval tv;

	pthread_t new_client_thread;

	/* loop with timeouts waiting for client connection requests and data */
	while (1) {

		/* check if new client is available for connection */
		if ( (client.socket == -1) && (new_client.socket != -1) ) {
			/* copy new client information */
			memcpy(&client, &new_client, sizeof(client_t));
			new_client.socket = -1;
			fprintf(stderr, "[%s] client %s connected\n", NAME, client.ip_string);
			/* ask client to provide a username before going to "character" mode */
			if (client_ask_username(&client) != 0) {
				/* close client if not able to provide a username */
				client_close(&client);
				continue;
			}
			/* put client in "character" mode */
			char msg[TELNET_MSG_LEN_CHARMODE];
			telnet_message_set_character_mode(msg);
			client_write(&client, msg, TELNET_MSG_LEN_CHARMODE);
		}

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
				/* handle new client connection request in a separate thread */
				resources_t r = {&server, &client, &new_client, &tty_dev};
				if (pthread_create(&new_client_thread, NULL, thread_new_client_connection, &r) != 0) {
					/* print error but continue waiting for connection request */
					fprintf(stderr, "[%s] problem with handling client connection request\n", NAME);
					continue;
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
				if (tty_dev.fd != -1) {
					tty_write(&tty_dev, client.data, ret);
				}
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

	return (void*) 0;
}

/* MoxaNix main program loop. */
int main(int argc, char *argv[]) {
	
	int ret;
	unsigned int tcp_port = -1;
	int def_conf = 0; 	// is default config used or from .cfg file

	pthread_t tty_thread;

	/* zero init tty_dev */
	if (cfsetispeed(&(tty_dev.ttyset), B0) < 0 || 
		cfsetospeed(&(tty_dev.ttyset), B0) < 0) {
		fprintf(stderr, "[%s] error configuring tty device speed\n", NAME);
		return -1;
	}
	
	/* enable catching and handling some quit signals, SIGKILL can't be caught */
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
			/* get tty device path, default config used */
			case 't':
				if ((strnlen(optarg, TTY_DEV_PATH_LEN) == 0) ||
					(strnlen(optarg, TTY_DEV_PATH_LEN) > (TTY_DEV_PATH_LEN - 1))) {
					fprintf(stderr, "[%s] error: tty path was not specified\n\n", NAME);
					usage();
					return -1;
				} else {
					/* set tty device path in tty_dev struct */
					strcpy(tty_dev.path, optarg);
				}
				def_conf = 1;
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

	/* initialize */
	if (server_setup(&server, tcp_port) < 0) return -1;
	client.socket = -1;
	new_client.socket = -1;
	tty_dev.fd = -1;
	
	/* parse config file if any */
	if (!def_conf && ((ret = ini_parse(CONFILE, &parse_handler, NULL)) == -1)) {
		fprintf(stderr, "[%s] error opening config file %s\n", NAME, CONFILE);
		usage();
		return -1;
	}
	else if (!def_conf && ret) {
		fprintf(stderr, "[%s] error parsing config file %s on line %d\n", NAME, CONFILE, ret);
		return -1;
	}
	
	if (!strcmp(tty_dev.path, "")) {
		fprintf(stderr, "[%s] error: no tty device path given for TCP port: %d\n"
				"\t\t-> check config file %s\n", NAME, tcp_port, CONFILE);
		return -1;
	}

	/* open tty device */
	if (tty_open(&tty_dev) < 0) {
		fprintf(stderr, "[%s] error: opening of tty device at %s failed\n"
				"\t\t-> continuing in echo mode\n", NAME, tty_dev.path); 
		debug_messages = 1;
	}
	
	fprintf(stderr, "[%s] TCP port: %d, TTY device path: %s\n", NAME, tcp_port, tty_dev.path);
	
	/* start thread that handles tty device */
	resources_t r = {&server, &client, &new_client, &tty_dev};
	ret = pthread_create(&tty_thread, NULL, thread_tty_data, &r);
	if (ret) {
		fprintf(stderr, "[%s] error starting serial monitor thread"
				", pthread_create returned %d\n", NAME, ret);
		return -1;
	}
	
	/* start thread function (in this thread) that handles client data */
	thread_client_data(NULL);
	
	/* unexpected break from client data loop */
	fprintf(stderr, "[%s] unexpected condition\n", NAME);
	/* cleanup and exit with -1 */
	cleanup(-1);
	
	return -1;
}
