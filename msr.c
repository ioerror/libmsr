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
#include "msr206.h"

/* Thanks Club Mate and h1kari! Toorcon 10 */

int
msr_cmd (int fd, uint8_t c)
{
	msr_cmd_t	cmd;
	
	cmd.msr_esc = MSR_ESC;
	cmd.msr_cmd = c;

	return (serial_write (fd, &cmd, sizeof(cmd)));
}

int
getstart (int fd)
{
	uint8_t b;
	int i, r;

	for (i = 0; i < 3; i++) {
			r = serial_readchar(fd, &b);
		if (b == MSR_RW_START)
			break;
	}

	if (i == 3)
		return (-1);

	return (0);
}

int
getend (int fd)
{
	msr_end_t m;

	serial_read (fd, &m, sizeof(m));

	if (m.msr_sts != MSR_STS_OK) {
		printf ("read returned error status: 0x%x\n", m.msr_sts);
		return (-1);
	}

	return (0);
}

int
comtest (int fd)
{
	int r;
	uint8_t buf[2];

	r = msr_cmd (fd, MSR_CMD_DIAG_COMM);

	if (r == -1)
   		err(1, "Commtest write failed");

	/*
	 * Read the result. Note: we're supposed to get back
	 * two characters: an escape and a 'y' character. But
	 * with my serial USB adapter, the escape sometimes
	 * gets lost. As a workaround, we scan only for the 'y'
	 * and discard the escape.
	 */

	while (1) {
		serial_readchar (fd, &buf[0]);
		if (buf[0] == MSR_STS_COMM_OK)
			break;
	}

	if (buf[0] != MSR_STS_COMM_OK) {
		printf("Communications test failure\n");
		return (-1);
	} else
		printf("Communications test passed.\n");
	
	return (0);
}

int 
get_firmware_version (int fd)
{
	uint8_t		buf[64];

	bzero (buf, sizeof(buf));

	msr_cmd (fd, MSR_CMD_FWREV);

	serial_readchar (fd, &buf[0]);

	/* read the result "REV?X.XX" */

	serial_read (fd, buf, 8);
	buf[8] = '\0';

	printf ("Firmware Version: %s\n", buf);

	return (0);
}

int
get_device_model (int fd)
{
	msr_model_t	m;

	msr_cmd (fd, MSR_CMD_MODEL);

	/* read the result as the value of X in "MSR206-X" */

	serial_read (fd, &m, sizeof(m));

	if (m.msr_s != MSR_STS_MODEL_OK)
		return (1);

	printf("Device Model: MSR-206-%c\n", m.msr_model);
	
	return (0);
}

int
flash_led (int fd, uint8_t led)
{

	int r;

	r = msr_cmd (fd, led);

	if (r == -1)
		err(1, "LED failure");

	usleep (100000);

	/* No response, look at the lights Dr. Love */
	return (0);
}

int 
do_leet_led_dance (int fd)
{
	int z = 0;
	while (z < 3) {
	flash_led(fd, MSR_CMD_LED_OFF);
	flash_led(fd, MSR_CMD_LED_GRN_ON);
	flash_led(fd, MSR_CMD_LED_YLW_ON);
	flash_led(fd, MSR_CMD_LED_RED_ON);
	usleep (100000);
	z++;
	}

	return(0);
}

int
gettrack_iso (int fd, int t, uint8_t * buf, uint8_t * len)
{
	uint8_t b;
	int i = 0;
	int l = 0;

	/* Start delimiter should be ESC <track number> */

	serial_readchar (fd, &b);
	if (b != MSR_ESC) {
		printf("NOT AN ESCAPE (%x)...\n", b);
		return (-1);
	}

	serial_readchar (fd, &b);
	if (b != t) {
		printf("NOT A TRACK (%x)...\n", b);
		return (-1);
	}

	while (1) {
		serial_readchar (fd, &b);
		if (b == MSR_ESC)
			break;
		printf("%c", b);
		/* Avoid overflowing the buffer */
		if (i < *len) {
			l++;
			buf[i] = b;
		}
		i++;
		if (b == MSR_RW_END)
			break;
	}


	if (b == MSR_RW_END) {
		*len = l;
		printf("\nTRACK %d READ FINISHED, GOT %d BYTES\n", t, l);
		return (0);
	} else {
		*len = 0;
		serial_readchar (fd, &b);
		if (b == MSR_RW_EMPTY)
			printf("TRACK %d READ IS EMPTY\n", t);
		if (b == MSR_RW_BAD)
			printf("TRACK %d READ ERROR\n", t);
	}

	return (-1);
}

int
gettrack_raw (int fd, int t, uint8_t * buf, uint8_t * len)
{
	uint8_t b, s;
	int i = 0;
	int l = 0;

	/* Start delimiter should be ESC <track number> */

	serial_readchar (fd, &b);
	if (b != MSR_ESC) {
		printf("NOT AN ESCAPE (%x)...\n", b);
		return (-1);
	}

	serial_readchar (fd, &b);
	if (b != t) {
		printf("NOT A TRACK (%x)...\n", b);
		return (-1);
	}

	serial_readchar (fd, &s);

	if (!s) {
		printf ("TRACK %d IS EMPTY\n", t);
		return (0);
	}

	for (i = 0; i < s; i++) {
		serial_readchar (fd, &b);
		printf("[%x]", b);
		/* Avoid overflowing the buffer */
		if (i < *len) {
			l++;
			buf[i] = b;
		}
	}

	printf ("\nREAD %d BYTES FROM TRACK %d\n", s, t);

	return (0);
}

int 
msr_sensor_test (int fd)
{
	uint8_t b[4];

	msr_cmd (fd, MSR_CMD_DIAG_SENSOR);
	
	printf("Attempting sensor test -- please slide a card...\n");

	serial_read (fd, &b, 2);

	if (b[0] == MSR_ESC && b[1] == MSR_STS_SENSOR_OK) {
		printf("Sensor test successfull\n");
		return 0;
	}

	printf("It appears that the sensor did not sense a magnetic card.\n");
	return (-1);
}

int
msr_ram_test (int fd)
{
	uint8_t b[2];

	printf("Running ram test...\n");
	msr_cmd (fd, MSR_CMD_DIAG_RAM);

	usleep (1000000);

	serial_read(fd, b, sizeof(b));

	if (b[0] == MSR_ESC && b[1] == MSR_STS_RAM_OK) {
		printf("RAM test successfull.\n");
 		return (0);
	} 
	
	printf("It appears that the RAM test failed\n");
	return (-1);
}

int
msr_set_hi_co (int fd)
{
	char b[2];

	printf("Putting the writer to Hi-Co mode...\n");

	msr_cmd (fd, MSR_CMD_SETCO_HI);
 
	/* read the result "<esc>0" if OK, unknown or no response if fail */
	serial_read (fd, &b, 2);
 
	if (b[0] == MSR_ESC && b[1] == MSR_STS_OK) {
		printf("We were able to put the writer into Hi-Co mode.\n");
		return (0);
	}
   
	printf("It appears that the reader did not switch to Hi-Co mode.");
	return (1);
}

int
msr_set_lo_co (int fd)
{
	char b[2];

	printf("Putting the writer to Lo-Co mode...\n");

	msr_cmd (fd, MSR_CMD_SETCO_LO);
 
	/* read the result "<esc>0" if OK, unknown or no response if fail */
	serial_read (fd, &b, 2);
 
	if (b[0] == MSR_ESC && b[1] == MSR_STS_OK) {
		printf("We were able to put the writer into Hi-Co mode.\n");
		return (0);
	}
   
	printf("It appears that the reader did not switch to Hi-Co mode.");
	return (1);
}

int
msr_reset (int fd)
{
	msr_cmd (fd, MSR_CMD_RESET);

	usleep (100000);

	return (0);
}

int
msr_iso_read(int fd, msr_tracks_t * tracks)
{
	int r; 

	r = msr_cmd (fd, MSR_CMD_READ);

	if (r == -1)
		err(1, "Command write failed");

	if (getstart (fd) == -1)
		err(1, "get start delimiter failed");

	gettrack_iso (fd, 1, tracks->msr_tk1_data, &tracks->msr_tk1_len);
	gettrack_iso (fd, 2, tracks->msr_tk2_data, &tracks->msr_tk2_len);
	gettrack_iso (fd, 3, tracks->msr_tk3_data, &tracks->msr_tk3_len);

	if (getend (fd) == -1)
		err(1, "read failed");

	return (0);
}

int
msr_raw_read(int fd, msr_tracks_t * tracks)
{
	int r; 

	r = msr_cmd (fd, MSR_CMD_RAW_READ);

	if (r == -1)
		err(1, "Command write failed");

	if (getstart (fd) == -1)
		err(1, "get start delimiter failed");

	gettrack_raw (fd, 1, tracks->msr_tk1_data, &tracks->msr_tk1_len);
	gettrack_raw (fd, 2, tracks->msr_tk2_data, &tracks->msr_tk2_len);
	gettrack_raw (fd, 3, tracks->msr_tk3_data, &tracks->msr_tk3_len);

	if (getend (fd) == -1)
		err(1, "read failed");

	return (0);
}

int main(int argc, char * argv[])
{
	int fd = -1;
	int serial;
	msr_tracks_t tracks;

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
	msr_reset (fd);

	/* Test the reader connection */
 	comtest (fd);

	/* Get the device model */
	get_device_model (fd);
	/* Get the firmware version information */
	get_firmware_version (fd);

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

	tracks.msr_tk1_len = MSR_MAX_TRACK_LEN;
	tracks.msr_tk2_len = MSR_MAX_TRACK_LEN;
	tracks.msr_tk3_len = MSR_MAX_TRACK_LEN;

	msr_iso_read (fd, &tracks);

	if (tracks.msr_tk1_len)
		printf ("track1: [%s]\n", tracks.msr_tk1_data);
	if (tracks.msr_tk2_len)
		printf ("track2: [%s]\n", tracks.msr_tk2_data);
	if (tracks.msr_tk3_len)
		printf ("track3: [%s]\n", tracks.msr_tk3_data);

	printf("Ready to read an raw formatted card. Please slide a card.\n");

	tracks.msr_tk1_len = MSR_MAX_TRACK_LEN;
	tracks.msr_tk2_len = MSR_MAX_TRACK_LEN;
	tracks.msr_tk3_len = MSR_MAX_TRACK_LEN;

	msr_iso_read (fd, &tracks);

	msr_reset (fd);

	/* We're finished */
	serial_close (fd);
	exit(0);
}
