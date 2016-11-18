#pragma once

/* Handles communication with a tty device. */

#include <common.h>
#include <client.h>
#include <termios.h>

#define DEV_PATH 128

typedef struct
{
	int fd;						/* tty file descriptor */
	struct termios ttysetdef;	/* default tty termios settings */
	struct termios ttyset;		/* tty termios settings */
	char path[DEV_PATH];		/* tty device path */
	char data[DATABUF_LEN];		/* buffer for data received from tty */
} tty_t;

/**
 * Opens the tty device and configures it.
 */
int tty_open(tty_t *tty_dev);

/**
 * Closes the tty device.
 */
int tty_close(tty_t *tty_dev);

/**
 * Reconfigures the tty device.
 */
int tty_reconfigure(tty_t *tty_dev, struct termios newttyset);

/**
 * Reads incoming data from tty device to tty data buffer.
 *
 * Returns:
 * - number of read bytes on success,
 * - negative errno value set by an error while readin
 */
int tty_read(tty_t *tty_dev);

/**
 * Sends data from a buffer to tty device.
 *
 * Returns:
 * - number of sent bytes on success,
 * - negative errno value set by an error while sending
 */
int tty_write(tty_t *tty_dev, char *databuf, int datalen);

/**
 * Converts POSIX speed_t to a baud rate.  The values of the
 * constants for speed_t are not themselves portable.
 */
int speed_to_baud(speed_t speed);

/**
 * Converts a numeric baud rate to a POSIX speed_t.
 */
speed_t baud_to_speed(int baud);
