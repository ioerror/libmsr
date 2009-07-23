#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

#define __USE_BSD
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>
#include <termios.h>
#include <err.h>
#include <string.h>

#include "libmsr.h"
#include "serialio.h"
#include "makstripe.h"

int
mak_cmd(int fd, uint8_t c)
{
	uint8_t buf[2];
	buf[0] = c;
	buf[1] = MAK_ESC;
	return serial_write(fd, &buf, 2);
}

int
mak_read(int fd)
{
	int r;
	r = mak_cmd(fd, MAKSTRIPE_READ);
	/* Ready response is needed here, check for this. */
	return r;
}

int
mak_write(int fd)
{
	int w;
	w = mak_cmd(fd, MAKSTRIPE_WRITE_BUF);
	return w;
}

int
mak_populate_buffer(int fd)
{
}

int
mak_clone(int fd)
{
	int c;
	c = mak_cmd(fd, MAKSTRIPE_CLONE);
	return c;
}
