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

#ifdef notdef
int
dumpbits (uint8_t * buf, int len)
{
	int		bytes, i;
	for (bytes = 0; bytes < len; bytes++) {
		/* for (i = 0; i < 8; i++) { */
		for (i = 7; i > -1; i--) {
			if (buf[bytes] & (1 << i))
				printf("1");
			else
				printf("0");
		}
	}
	printf ("\n");
	return (0);
}

int
getbit (uint8_t * buf, uint8_t len, int bit)
{
	int		byte;
	int		b;

	if (bit > (len * 8)) {
		printf ("%d > %d\n", bit, len * 8);
		return (-1);
	}
	byte = bit / 8;
	b = 7 - (bit % 8);
	if (buf[byte] & (1 << b))
		return (1);
	return (0);
}

int
setbit (uint8_t * buf, uint8_t len, int bit, int val)
{
	int		byte;
	int		b;

	if (bit > (len * 8)) {
		printf ("%d > %d\n", bit, len * 8);
		return (-1);
	}
	byte = bit / 8;
	b = 7 - (bit % 8);

	if (val)
		buf[byte] |= 1 << b;
	else
		buf[byte] &= ~(1 << b);

	return (0);
}
#endif

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
	uint8_t buf[256];
	uint8_t len;

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

	serial = serial_open (device, &fd);

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

	msr_set_bpi(fd, 210);
	msr_init (fd);
	msr_set_bpc(fd, 7, 5, 7);

	bzero ((char *)&tracks, sizeof(tracks));

	for (i = 0; i < MSR_MAX_TRACKS; i++)
		tracks.msr_tracks[i].msr_tk_len = MSR_MAX_TRACK_LEN;

	printf("Ready to do a raw read. Please slide a card.\n");
	msr_raw_read (fd, &tracks);

	for (i = 0; i < MSR_MAX_TRACKS; i++) {
		int x;
		printf("track%d [%d]: ", i, tracks.msr_tracks[i].msr_tk_len);
		for (x = 0; x < tracks.msr_tracks[i].msr_tk_len; x++)
			printf("%x ", tracks.msr_tracks[i].msr_tk_data[x]);
		printf("\n");
#ifdef notdef
		dumpbits (tracks.msr_tracks[i].msr_tk_data,
		    tracks.msr_tracks[i].msr_tk_len);
		for (x = 0; x < 32; x++)
			printf ("%d", getbit (tracks.msr_tracks[i].msr_tk_data,
			    tracks.msr_tracks[i].msr_tk_len, x));
		printf("\n");
#endif
	}

#ifdef notdef
	{
		uint8_t * b;
		uint8_t len;
		int ch = 0;
		int bits = 0;
		char byte = 0;

		len = tracks.msr_tracks[1].msr_tk_len;
		b = tracks.msr_tracks[1].msr_tk_data;
printf ("LEN: %d\n", len);
		for (i = 0; i < len * 8; i++) {
			byte |= msr_getbit (b, len, bits) << ch;
			if (ch == 4) {
				printf ("%c", (byte & ~0x20) | 0x30);
				ch = 0;
				byte = 0;
			} else
				ch++;
			bits++;
		}
		printf ("\n");
	}
#endif

	len = 255;
	bzero (buf, sizeof(buf));
	msr_decode (tracks.msr_tracks[0].msr_tk_data,
	    tracks.msr_tracks[0].msr_tk_len, buf, &len, 7);
	printf("[%d] %s\n", len, buf);
	len = 255;
	bzero (buf, sizeof(buf));
	msr_decode (tracks.msr_tracks[1].msr_tk_data,
	    tracks.msr_tracks[1].msr_tk_len, buf, &len, 5);
	printf("[%d] %s\n", len, buf);
	len = 255;
	bzero (buf, sizeof(buf));
	msr_decode (tracks.msr_tracks[2].msr_tk_data,
	    tracks.msr_tracks[2].msr_tk_len, buf, &len, 5);
	printf("[%d] %s\n", len, buf);

	msr_init (fd);

	/* We're finished */
	serial_close (fd);
	exit(0);
}
