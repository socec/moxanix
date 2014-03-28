/* 
 * Handling communication with tty device.
 */

#include "moxerver.h"

#define TTY_THREAD_TIMEOUT_SEC 30
#define TTY_WAIT_TIMEOUT 5 /* seconds for select() timeout in server loop */
#define NAME "tty"
#define TTY_DEF_BAUD_RATE B115200

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

	tty_dev->ttyset.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR |
							 	 PARMRK | INPCK | ISTRIP | IXON);
	tty_dev->ttyset.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
							 	 ONOCR | OFILL | OLCUC | OPOST);
	tty_dev->ttyset.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	tty_dev->ttyset.c_cflag &= ~(CSIZE | PARENB);
	tty_dev->ttyset.c_cflag |= CS8;
	tty_dev->ttyset.c_cc[VMIN]  = 1;
	tty_dev->ttyset.c_cc[VTIME] = 0;

	/* if speed is set to B0 (e.g. cfg file is not provided), default values are used */
	if (cfgetispeed(&(tty_dev->ttyset)) == baud_to_speed(0) && 
		cfsetispeed(&(tty_dev->ttyset), TTY_DEF_BAUD_RATE) < 0) {
		fprintf(stderr, "[%s] error configuring tty device speed\n", NAME);
		return -errno;
	}
	if (cfgetospeed(&(tty_dev->ttyset)) == baud_to_speed(0) && 
		cfsetospeed(&(tty_dev->ttyset), TTY_DEF_BAUD_RATE) < 0) {
		fprintf(stderr, "[%s] error configuring tty device speed\n", NAME);
		return -errno;
   	}

   	if(tcsetattr(tty_dev->fd, TCSAFLUSH, &(tty_dev->ttyset)) < 0) {
		fprintf(stderr, "[%s] error configuring tty device\n", NAME);
		return -errno;
   	}

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
	write(tty_dev->fd, databuf, datalen);
	// databuf should point to client data buffer
	return 0;
}

/* Main thread for reading and writing to tty device */
void *tty_thread_func(void *arg) {
	//char c;
	struct tty_t *tty_dev = (struct tty_t*)arg;	
	struct timeval tv;
	ssize_t br = 0;
	int ret;
	fd_set read_fds;

	fprintf(stderr, "[%s] tty thread started with passed argument: %s\n", NAME, tty_dev->path);

	while (1) {
		/* setup parameters for select() */
		tv.tv_sec = TTY_WAIT_TIMEOUT;
		tv.tv_usec = 0;
		FD_ZERO(&read_fds);
		FD_SET(tty_dev->fd, &read_fds);
			
		/* wait with select() */
		ret = select(tty_dev->fd + 1, &read_fds, NULL, NULL, &tv);

		if (ret > 0 && FD_ISSET(tty_dev->fd, &read_fds)) {
			br = read(tty_dev->fd, tty_dev->data, DATABUF_LEN);
			client_write(&client, tty_dev->data, br);
		}
		else {
		}
		//sleep(10);
		//if (read(tty_dev->fd, &c, 1) > 0)        
		//	printf("%c", c); 
	     	                                	
		//fprintf(stderr, "[%s] tty thread reporting ...\n", NAME);
			//i++;
	}

	fprintf(stderr, "[%s] tty thread stoped\n", NAME);

	return (void *)tty_dev;;
}

/*
 * Converts POSIX speed_t to a baud rate.  The values of the
 * constants for speed_t are not themselves portable.
 */
int speed_to_baud(speed_t speed)
{
	switch (speed) {
	case B0:
		return 0;
	case B50:
		return 50;
	case B75:
		return 75;
	case B110:
		return 110;
	case B134:
		return 134;
	case B150:
		return 150;
	case B200:
		return 200;
	case B300:
		return 300;
	case B600:
		return 600;
	case B1200:
		return 1200;
	case B1800:
		return 1800;
	case B2400:
		return 2400;
	case B4800:
		return 4800;
	case B9600:
		return 9600;
	case B19200:
		return 19200;
	case B38400:
		return 38400;
	case B57600:
		return 57600;
	case B115200:
		return 115200;
	default:
		return 115200;
	}
}

/*
 * Converts a numeric baud rate to a POSIX speed_t.
 */
speed_t baud_to_speed(int baud)
{
	switch (baud) {
	case 0:
		return B0;
	case 50:
		return B50;
	case 75:
		return B75;
	case 110:
		return B110;
	case 134:
		return B134;
	case 150:
		return B150;
	case 200:
		return B200;
	case 300:
		return B300;
	case 600:
		return B600;
	case 1200:
		return B1200;
	case 1800:
		return B1800;
	case 2400:
		return B2400;
	case 4800:
		return B4800;
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	default:
		return B115200;
	}
}
