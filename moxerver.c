/* 
 * Main server application.
 * Handles client connections on specific TCP port and allows bidirectional
 * communication with a specific TTY device.
 */

#include <common.h>
#include <task_threads.h>
#include <parser.h>
#include <signal.h> /* handling quit signals */

#define NAME "moxerver"

#define CONFILE "moxanix.cfg"

/* Global variables used throughout the application. */
server_t server;	 /* main server */
client_t client;	 /* connected client */
client_t new_client; /* reserved for a new client request */
tty_t tty_dev;		 /* connected tty device */

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

/* MoxaNix main program loop. */
int main(int argc, char *argv[])
{
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
	
	/* start thread function that handles tty device */
	resources_t r = {&server, &client, &new_client, &tty_dev};
	ret = pthread_create(&tty_thread, NULL, thread_tty_data, &r);
	if (ret) {
		fprintf(stderr, "[%s] error starting serial monitor thread"
				", pthread_create returned %d\n", NAME, ret);
		return -1;
	}
	
	/* start thread function (in this thread) that handles client data */
	thread_client_data(&r);
	
	/* unexpected break from client data loop */
	fprintf(stderr, "[%s] unexpected condition\n", NAME);
	/* cleanup and exit with -1 */
	cleanup(-1);
	
	return -1;
}
