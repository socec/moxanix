#include "moxerver.h"
#include <string.h>
#define TTY_THREAD_TIMEOUT_SEC 30
#define TTY_WAIT_TIMEOUT 5 /* seconds for select() timeout in server loop */
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

	tty_dev->ttyset.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR |
								 PARMRK | INPCK | ISTRIP | IXON);
	tty_dev->ttyset.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
								 ONOCR | OFILL | OLCUC | OPOST);
	tty_dev->ttyset.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	tty_dev->ttyset.c_cflag &= ~(CSIZE | PARENB);
	tty_dev->ttyset.c_cflag |= CS8;
	tty_dev->ttyset.c_cc[VMIN]  = 1;
	tty_dev->ttyset.c_cc[VTIME] = 0;

	if(cfsetispeed(&(tty_dev->ttyset), B115200) < 0 || cfsetospeed(&(tty_dev->ttyset), B115200) < 0) {
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
			br = read(tty_dev->fd, tty_dev->data, DATA_BUFLEN);
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

