#include "moxerver.h"


/* Opens the tty device and configures it. */
int tty_open(struct tty_t *tty_dev, char* path) {
	// PROPOSAL:
	// open tty device to get file descriptor @tty_dev.fd
	// setup tty device parameters @tty_dev.ttyset
	// apply settings by calling tcsetattr(fd, ttyset)
	// on success copy path to @tty_dev.path
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