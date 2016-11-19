/* Handles communication with a tty device. */

#pragma once

#include <common.h>
#include <client.h>
#include <termios.h>

#define TTY_DEV_PATH_LEN 128

typedef struct
{
	int fd;						 /* tty device file descriptor */
	struct termios ttysetold;	 /* previous termios settings */
	struct termios ttyset;		 /* current termios settings */
	char path[TTY_DEV_PATH_LEN]; /* tty device path */
	char data[BUFFER_LEN];		 /* buffer for received data */
} tty_t;

/**
 * Opens the tty device and configures it.
 * The old device settings are saved.
 *
 * Returns:
 * - 0 on success
 * - negative errno value if an error occurred
 */
int tty_open(tty_t *tty_dev);

/**
 * Closes the tty device connection.
 * Also applies the old device settings.
 *
 * Returns:
 * - 0 on success
 * - negative errno value if an error occurred
 */
int tty_close(tty_t *tty_dev);

/**
 * Reads incoming data from tty device to tty data buffer.
 *
 * Returns:
 * - number of read bytes on success,
 * - negative errno value set by an error while reading
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
 * Converts POSIX speed_t to a baud rate.
 * The values of the constants for speed_t are not themselves portable.
 */
int speed_to_baud(speed_t speed);

/**
 * Converts a numeric baud rate to a POSIX speed_t.
 */
speed_t baud_to_speed(int baud);
