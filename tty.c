#include "moxerver.h"


/* Opens the tty device and configures it. */
int tty_open(struct tty_t *tty_dev) {
	return 0;
}
/* Closes the tty device. */
int tty_close(struct tty_t *tty_dev) {
	return 0;
}

/* Reconfigures the tty device. */
int tty_reconfigure(struct tty_t *tty_dev, struct termios newttyset) {
	return 0;
}

/* Reads incoming data from tty device to tty data buffer. */
int tty_read(struct tty_t *tty_dev) {
	return 0;
}

/* Sends data from a buffer to tty device. */
int tty_write(struct tty_t *tty_dev, char *databuf, int datalen) {
	return 0;
}