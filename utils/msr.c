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

int 
do_leet_led_dance (int fd)
{
	int z = 0;
	while (z < 3) {
	msr_flash_led(fd, MSR_CMD_LED_OFF);
	msr_flash_led(fd, MSR_CMD_LED_GRN_ON);
	msr_flash_led(fd, MSR_CMD_LED_YLW_ON);
	msr_flash_led(fd, MSR_CMD_LED_RED_ON);
	usleep (100000);
	z++;
	}

	return(0);
}

int main(int argc, char * argv[])
{
	int fd = -1;
	int serial;
	msr_tracks_t tracks;
	int i;

	/* Default device selection per platform */
#ifdef __linux__ 
	char *device = "/dev/ttyS0";
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

	/* Test the mag sensor */
	msr_sensor_test (fd);

	/* Ram test */
	msr_ram_test (fd);

	/* Flash the LEDs to make things more 31337 */
	printf("Preparing reader for reading...\n");
	do_leet_led_dance (fd);

	printf("Ready to read an ISO formatted card. Please slide a card.\n");

	/* Now we'll tell the reader we'd like to read the ISO formatted data */
	bzero ((char *)&tracks, sizeof(tracks));

	for (i = 0; i < MSR_MAX_TRACKS; i++)
		tracks.msr_tracks[i].msr_tk_len = MSR_MAX_TRACK_LEN;

	msr_iso_read (fd, &tracks);

	for (i = 0; i < MSR_MAX_TRACKS; i++) {
		if (tracks.msr_tracks[i].msr_tk_len)
			printf ("track%d: [%s]\n", i,
				tracks.msr_tracks[i].msr_tk_data);
	}

	/* Prepare the reader with a reset */
	msr_init (fd);

	printf ("Attempting erase, please swipe card...\n");
	msr_erase (fd, MSR_ERASE_ALL);

	/* Prepare the reader with a reset */
	msr_init (fd);

	printf ("Attempting ISO write, please swipe card...\n");

#ifdef notdef
	tracks.msr_tracks[0].msr_tk_len = 3;
	strcpy (tracks.msr_tracks[0].msr_tk_data, "MOO");
	tracks.msr_tracks[1].msr_tk_len = 3;
	strcpy (tracks.msr_tracks[1].msr_tk_data, "123");
	tracks.msr_tracks[2].msr_tk_len = 3;
	strcpy (tracks.msr_tracks[2].msr_tk_data, "456");
#endif

	msr_iso_write (fd, &tracks);

	printf ("write complete\n");

#ifdef notdef
	bzero ((char *)&tracks, sizeof(tracks));

	for (i = 0; i < MSR_MAX_TRACKS; i++)
		tracks.msr_tracks[i].msr_tk_len = MSR_MAX_TRACK_LEN;

	printf("Ready to do a raw read. Please slide a card.\n");
	msr_raw_read (fd, &tracks);

	for (i = 0; i < MSR_MAX_TRACKS; i++) {
		int x;
		printf("track%d: ", i);
		for (x = 0; x < tracks.msr_tracks[i].msr_tk_len; x++)
			printf("%x ", tracks.msr_tracks[i].msr_tk_data[x]);
		printf("\n");
	}
#endif

	msr_init (fd);

	/* We're finished */
	serial_close (fd);
	exit(0);
}
