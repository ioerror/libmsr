#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <err.h>

#include "libmsr.h"

int
getstart (int fd)
{
	char b;
	int i, r;

	for (i = 0; i < 3; i++) {
		r = read (fd, &b, 1);
		if (b == MSR_STARTDELIM)
			break;
	}

	if (i == 3)
		return (-1);

	return (0);
}

int
gettrack (int fd, int t)
{
	char b;

	/* Start delimiter should be ESC <track number> */

	read (fd, &b, 1);
	if (b != MSR_ESC) {
		printf("NOT AN ESCAPE (%x)...\n", b);
		return (-1);
	}
	read (fd, &b, 1);
	if (b != t)
		{
		printf("NOT A TRACK (%x)...\n", b);
		return (-1);
		}

	printf("GETTING TRACK %d\n", t);
	while (1) {
		read (fd, &b, 1);
		if (b == MSR_ENDDELIM1)
			break;
		printf("%c", b);
	}

	printf("\n");
	/* Should be another escape here */
	if (b == MSR_ENDDELIM1) {
		printf("READ TRACK %d\n", t);
		return (0);
	}
	return (-1);
}

int main()
{
	int		fd;
    struct termios	options;
	char		buf[256];
	char		b;
	int		i, r;
	int		stop = 0;

    // Default device selection per platform
    #ifdef linux 
        const char *device = "/dev/ttyS0";
    #elif BSD
        const char *device = "/dev/cuaU0";
    #endif

	bzero (buf, sizeof(buf));

    printf("Attempting to open %s\n", device);

	fd = open(device, O_RDWR | O_NOCTTY /*| O_NDELAY*/);

	if (fd == -1)
		err(1, "open failed");

	printf("FD: %d\n", fd);

	tcgetattr(fd, &options);

	printf("got options...\n");

	/* Set baud rate */

	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;

	options.c_cflag |= CS8;

	options.c_lflag &= ~ICANON;
	options.c_lflag &= ~ICRNL;
	options.c_lflag &= ~ECHO;
	options.c_iflag |= IXOFF;

	options.c_oflag &= ~OPOST;
	tcsetattr(fd, TCSANOW, &options);
	printf("set options...\n");

	buf[0] = MSR_ESC;
	buf[1] = MSR_RESET;

	r = write (fd, buf, 2);

	usleep (100000);

	buf[0] = MSR_ESC;
	buf[1] = MSR_ISO_READ; 

	r = write (fd, buf, 2);

	usleep (100000);

	if (fd == -1)
		err(1, "write failed");
	else
		printf("wrote: %d\n", r);

	i = 0;

	if (getstart (fd) == -1)
		err(1, "get start delimiter failed");

	gettrack (fd, 1);
	gettrack (fd, 2);
	gettrack (fd, 3);

	close (fd);

	printf("buf: %s\n", buf);

	exit(0);
}
