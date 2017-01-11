#include <tty.h>

#define TTY_DEFAULT_BAUDRATE B115200

int tty_open(tty_t *tty_dev)
{
	/* open tty device to get the file descriptor */
	tty_dev->fd = open (tty_dev->path, O_RDWR | O_NOCTTY | O_SYNC);
	if (tty_dev->fd < 0)
	{
		tty_dev->fd = -1;
		return -errno;
	}

	/* store default termios settings */
	if (tcgetattr(tty_dev->fd, &(tty_dev->ttysetold)))
	{
		LOG("[@%d] error reading device default config\n"
			"\t\t-> default config will not be restored upon exit", __LINE__);
	}

	/* set tty device parameters */
	tty_dev->ttyset.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR |
							 	 PARMRK | INPCK | ISTRIP | IXON);
	tty_dev->ttyset.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
							 	 ONOCR | OFILL | OLCUC | OPOST);
	tty_dev->ttyset.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	tty_dev->ttyset.c_cflag &= ~(CSIZE | PARENB);
	tty_dev->ttyset.c_cflag |= CS8 | CREAD;
	tty_dev->ttyset.c_cc[VMIN]  = 1;
	tty_dev->ttyset.c_cc[VTIME] = 5;

	/* if speed is set to B0 (e.g. cfg file not provided), use default values */
	if (cfgetispeed(&(tty_dev->ttyset)) == baud_to_speed(0) && 
		cfsetispeed(&(tty_dev->ttyset), TTY_DEFAULT_BAUDRATE) < 0)
	{
		LOG("error configuring tty device speed");
		return -errno;
	}
	if (cfgetospeed(&(tty_dev->ttyset)) == baud_to_speed(0) && 
		cfsetospeed(&(tty_dev->ttyset), TTY_DEFAULT_BAUDRATE) < 0)
	{
		LOG("error configuring tty device speed");
		return -errno;
   	}

	/* apply tty device settings */
   	if (tcsetattr(tty_dev->fd, TCSANOW, &(tty_dev->ttyset)) < 0)
   	{
		LOG("error configuring tty device");
		return -errno;
   	}

	return 0;
}

int tty_close(tty_t *tty_dev)
{
	int fd = tty_dev->fd;
	tty_dev->fd = -1;

	LOG("closing tty device");
	
	if (tcsetattr(fd, TCSANOW, &(tty_dev->ttysetold)) < 0)
	{
		LOG("[@%d] error restoring tty device default config", __LINE__);
		return -errno;
   	}

	if (close(fd) < 0)
	{
		return -errno;
	}

	return 0;
}

int tty_read(tty_t *tty_dev)
{
	int len;

	len = read(tty_dev->fd, tty_dev->data, BUFFER_LEN);
	if (len == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
		return -errno;
	}

	//TODO let's print received bytes during development phase...
	if (debug_messages)
	{
		int i;
		for(i = 0; i < len; i++)
		{
			LOG("tty <- %u '%c'",
				(unsigned char) tty_dev->data[i],
				(unsigned char) tty_dev->data[i]);
		}
	}

	return len;
}

int tty_write(tty_t *tty_dev, char *databuf, int datalen)
{
	int len;

	len = write(tty_dev->fd, databuf, datalen);
	if (len == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
		return -errno;
	}

	//TODO let's print received bytes during development phase...
	if (debug_messages)
	{
		int i;
		for(i = 0; i < datalen; i++)
		{
			LOG("tty -> %u '%c'",
				(unsigned char) databuf[i],
				(unsigned char) databuf[i]);
		}
	}

	return len;
}

int speed_to_baud(speed_t speed)
{
	switch (speed)
	{
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

speed_t baud_to_speed(int baud)
{
	switch (baud)
	{
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
