#include "moxerver.h"
#include <string.h>
#define TTY_THREAD_TIMEOUT_SEC 30
#define NAME "tty"

/* Opens the tty device and configures it. */
int tty_open(struct tty_t *tty_dev) {
	int fd;
	// PROPOSAL:
	// open tty device to get file descriptor @tty_dev.fd
	// setup tty device parameters @tty_dev.ttyset
	// apply settings by calling tcsetattr(fd, ttyset)
	// on success copy path to @tty_dev.path
	if ((fd = open (tty_dev->path, O_RDWR | O_NOCTTY | O_SYNC)) < 0)
		return -errno;
	else 
		tty_dev->fd = fd;
		
	return 0;
	
}
/* Closes the tty device. */
int tty_close(struct tty_t *tty_dev) {
	// close and set "tty_dev.fd = -1"
	return 0;
}

/* Reconfigures the tty device. */
int tty_reconfigure(struct tty_t *tty_dev, struct termios newttyset) {
	// not sure how to organize this:
	// 1. parameters in external termios struct, copied @tty_dev.ttyset, applied with tcsetattr()
	// 2. parameters directly @tty_dev.ttyset, applied with tcsetattr()
	return 0;
}

/* Reads incoming data from tty device to tty data buffer. */
int tty_read(struct tty_t *tty_dev) {
	// read and save @tty_dev.data
	return 0;
}

/* Sends data from a buffer to tty device. */
int tty_write(struct tty_t *tty_dev, char *databuf, int datalen) {
	// databuf should point to client data buffer
	return 0;
}

void *tty_thread_func(void *arg) {
	int i = 0;
	char *str;
	str = (char*)arg;

	fprintf(stderr, "[%s] tty thread started with passed argument: %s\n", NAME, str);

	//while ((i * 10) < TTY_THREAD_TIMEOUT_SEC) {
	while (1) {
		sleep(10);
		fprintf(stderr, "[%s] tty thread reporting ...\n", NAME);
		i++;
	}

	fprintf(stderr, "[%s] tty thread stoped\n", NAME);

	strncpy(str, "bye", strlen(str));
	
	return (void *)str;
}

