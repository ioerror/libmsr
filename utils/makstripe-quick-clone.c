#include <sys/types.h>

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

int main(int argc, char * argv[])
{
	int fd = -1;
	int serial;

	/* Default device selection per platform */
#ifdef __linux__
	char *device = "/dev/ttyUSB0";
#else
	char *device = "/dev/cuaU0";
#endif

	if (argv[1] != NULL)
		device = argv[1];
	else
		printf ("no device specified, defaulting to %s\n", device);

	serial = serial_open (device, &fd);

	if (serial == -1) {
		err(1, "Serial open of %s failed", device);
		exit(1);
	}

	/* The MAKStripe doesn't appear to need a reset at start up. */
	printf ("Attempting to clone card.\n");
	mak_clone(fd);

	/* We're finished */
	serial_close (fd);
	exit(0);
}
