/* 
 * Main server application.
 * Handles client connections on specific TCP port and allows bidirectional
 * communication with a specific TTY device.
 */

#include <common.h>
#include <task_threads.h>
#include <signal.h> /* handling quit signals */

/* ========================================================================== */

/* global resources */
server_t server;	 /* main server */
client_t client;	 /* connected client */
client_t new_client; /* reserved for a new client request */
tty_t tty_dev;		 /* connected tty device */

/* ========================================================================== */

/* Prints the help message. */
static void usage()
{
	//TODO maybe some styling should be done
	fprintf(stdout, "Usage: %s -p tcp_port -t tty_path -b baud_rate [-d] [-h]\n", APPNAME);
	fprintf(stdout, "\t-d\tturns on debug messages\n");
	fprintf(stdout, "\n");
}

/* Performs resource cleanup. */
void cleanup()
{
	LOG("performing cleanup");

	// TODO: maybe pthread_kill() should be used for thread cleanup?

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
}

/* Handles received quit signals, use it for all quit signals of interest. */
void quit_handler(int signum)
{
    /* perform cleanup and exit with 0 */
	LOG("received signal %d", signum);
	exit(0);
}

void time2string(time_t time, char* timestamp)
{
	strftime(timestamp, TIMESTAMP_LEN, TIMESTAMP_FORMAT, localtime(&time));
}

/* MoxaNix main program loop. */
int main(int argc, char *argv[])
{
	int ret;
	unsigned int tcp_port = -1;

	pthread_t tty_thread;

	/* initialize tty_dev */
	if (cfsetispeed(&(tty_dev.ttyset), B0) < 0 ||
		cfsetospeed(&(tty_dev.ttyset), B0) < 0)
	{
		LOG("error configuring tty device speed");
		return -1;
	}
	
	/* enable catching and handling some quit signals, SIGKILL can't be caught */
	signal(SIGTERM, quit_handler);
	signal(SIGQUIT, quit_handler);
	signal(SIGINT, quit_handler);
	
	/* check argument count */
	if (argc <= 1)
	{
		usage();
		return -1;
	}
	/* grab arguments */
	debug_messages = 0;
	while ((ret = getopt(argc, argv, ":p:t:b:dh")) != -1)
	{
		size_t path_len;
		speed_t baudrate;
		switch (ret)
		{
			/* get server port number */
			case 'p':
				tcp_port = (unsigned int) atoi(optarg);
				if (tcp_port < 0)
				{
					LOG("error, invalid TCP port value\n");
					usage();
					return -1;
				}
				break;
			/* get tty device path */
			case 't':
				path_len = strnlen(optarg, TTY_DEV_PATH_LEN);
				/* check correct path size */
				if ((path_len == 0) || (path_len > (TTY_DEV_PATH_LEN - 1)))
				{
					LOG("error with tty path length: should be <%d\n", TTY_DEV_PATH_LEN);
					usage();
					return -1;
				}
				/* otherwise, set tty device path in tty_dev struct */
				else
				{
					strcpy(tty_dev.path, optarg);
				}
				break;
			/* get tty device baud rate */
			case 'b':
				baudrate = baud_to_speed(atoi(optarg));
				if (cfsetispeed(&(tty_dev.ttyset), baudrate) < 0 ||
					cfsetospeed(&(tty_dev.ttyset), baudrate) < 0)
				{
					LOG("error configuring tty device baud rate, check configuration");
					return -1;
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
				LOG("error parsing arguments");
				usage();
				return -1;
		}
	}

	/* start server */
	client.socket = -1;
	new_client.socket = -1;
	if (server_setup(&server, tcp_port) < 0)
	{
		return -1;
	}

	/* open tty device */
	tty_dev.fd = -1;
	if (tty_open(&tty_dev) < 0)
	{
		LOG("error: opening of tty device at %s failed\n"
			"\t\t-> continuing in echo mode", tty_dev.path);
		debug_messages = 1;
	}
	
	LOG("Running with TCP port: %d, TTY device path: %s", tcp_port, tty_dev.path);

	/* start thread function that handles tty device */
	resources_t r = {&server, &client, &new_client, &tty_dev};
	ret = pthread_create(&tty_thread, NULL, thread_tty_data, &r);
	if (ret) {
		LOG("error starting serial monitor thread, pthread_create returned %d", ret);
		return -1;
	}
	
	/* start thread function (in this thread) that handles client data */
	thread_client_data(&r);
	
	/* unexpected break from client data loop, cleanup and exit with -1 */
	LOG("unexpected condition");
	cleanup();
	return -1;
}
