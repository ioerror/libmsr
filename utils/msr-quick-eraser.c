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
#include "msr206.h"

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

	serial = serial_open (device, &fd, MSR_BLOCKING, MSR_BAUD);

	if (serial == -1) {
		err(1, "Serial open of %s failed", device);
		exit(1);
	}

	/* Prepare the reader with a reset */
	msr_init (fd);

	/* Get the device model */
	msr_model (fd);
	/* Get the firmware version information */
	msr_fwrev (fd);

	msr_set_lo_co (fd);

	/* Prepare the reader with a reset */
	msr_init (fd);

	printf ("Attempting erase, please swipe card...\n");
	msr_erase (fd, MSR_ERASE_ALL);

	msr_init (fd);

	/* We're finished */
	serial_close (fd);
	exit(0);
}
