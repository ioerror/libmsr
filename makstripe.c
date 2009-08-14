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

/* I can't believe that I'm so lazy */
#include <arpa/inet.h>


#include "libmsr.h"
#include "serialio.h"
#include "makstripe.h"

/* Remember that the MAKStripe desires MAK_BAUD for serial io. */
/* It also requires MAK_BLOCK which we haven't defined. */

int
mak_cmd(int fd, uint8_t c, uint8_t tracks)
{
	mak_generic_cmd_t	cmd;
	cmd.mak_cmd = c;
	cmd.mak_track_mask = tracks;
	printf("Attempting to send command: %c\n", cmd.mak_cmd);
	return (serial_write(fd,&cmd, sizeof(cmd)));
}

int mak_reset(int fd)
{
	int r;
	int i;
	char buf[strlen(MAK_RESET_RESP) + 1];
	memset(buf, 0, strlen(MAK_RESET_RESP));
	buf[0] = MAK_RESET_CMD;
	printf("Sending reset command: %c\n", MAK_RESET_CMD);
	serial_write(fd, buf, sizeof(MAK_RESET_CMD));
	printf("We expect: %s\n", MAK_RESET_RESP);
	printf("We got ");
	r = serial_read(fd, buf, strlen(MAK_RESET_RESP));
	buf[strlen(MAK_RESET_RESP)] = '\0';
	printf("buf: %s\n", buf);
	printf("Reset status: %d\n", r);
	return r;
}

int mak_flush(fd)
{
	int r;
	char buf[1];
	do {
		r = serial_read(fd, buf, 1);
	} while (r != 0);
	return r;
}

int
mak_read(int fd, uint8_t tracks)
{
	int r;
	int i;
	char buf[5];
	unsigned char sample_tmp[2];
	uint16_t sample_count;
	int sample_count_guessing;
	printf("Attempting to perform a read...\n");

	r = mak_cmd(fd, MAKSTRIPE_READ_CMD, tracks);

	printf("Command sent!\n");
	/* Ready response is needed here, check for this. */
	serial_read(fd, buf, strlen(MAKSTRIPE_READ_RESP) -1);
	printf("We got: %s\n", buf);
	printf("We expected: %s\n", MAKSTRIPE_READ_RESP);
	if (!memcmp(buf, MAKSTRIPE_READ_RESP, strlen(MAKSTRIPE_READ_RESP))) {
		printf("Doh!\n");
		return -1;
	}
	printf("Please swipe a card for read...\n");

	memset(buf, 0, 3);
	/* 'RD '<16bits of length data><data samples> */
	serial_read(fd, buf, 3);
	printf("We expect: %s\n", MAKSTRIPE_READ_BUF_PREFIX);
	printf("We expect it to be of len: %d\n", strlen(MAKSTRIPE_READ_BUF_PREFIX));
	printf("We got: %s\n", buf);
	r = memcmp(buf, MAKSTRIPE_READ_BUF_PREFIX, strlen(MAKSTRIPE_READ_BUF_PREFIX));
	if (r != 0) {
		return -1;
	}
	serial_read(fd, &sample_count, 2);
	printf("Sample count appears to be: %d\n", sample_count);
	sample_count_guessing = ntohs(sample_count);
	printf("Sample count appears to be: %d\n", sample_count_guessing);

	/* XXX: We currently clobber the data. We don't return it. */
	for (i = 0; i < sample_count_guessing ; i++) { /* Why is this off? sleeppppy... */
		serial_read(fd, sample_tmp, 2);
		printf("%d %02x %02x\n", i, sample_tmp[0], sample_tmp[1]);
	}

	printf("In theory, we have dumped the full sample data now...\n");
	/* Lets ensure that the sample read went correctly! */
	serial_read(fd, buf, 5);
	printf("Sample read returned status: %s\n", buf);

	r = memcmp(buf, MAKSTRIPE_READ_STS_OK, 5);
	if (r != 0)
		return -1;

	return r;
}

/* The MAKStripe is a bit of a pain and has failures reading often. Wrap it.*/
int
mak_successful_read(int fd, uint8_t tracks)
{
	int r;
	do {
		mak_reset(fd);
		r = mak_read(fd, tracks);
	} while (r != 0);
	return r;
}

/*
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

*/

/*
 * Send: C<0x7>
 * Response: CP<space>
 * <swipe card>
 * Response: CP=OK
*/
int
mak_clone(int fd)
{
	int c;
	char buf[strlen(MAKSTRIPE_CLONE_STS_OK)];
	buf[0] = MAKSTRIPE_CLONE_CMD;
	buf[1] = 0x7;
	c = serial_write(fd, buf, 2);
	printf("Serial write status: %i\n", c);

/*
	if (c != 0) {
		printf("Clone command failure.\n");
		return -1;
	}
*/

	/* Read the response and make sure it matches MAKSTRIPE_CLONE_RESP */
	c = serial_read(fd, buf, strlen(MAKSTRIPE_CLONE_RESP));
	if (memcmp(buf, MAKSTRIPE_CLONE_RESP, strlen(MAKSTRIPE_CLONE_RESP)) != 0) {
		printf("buf should be: %s\n", MAKSTRIPE_CLONE_RESP);
		printf("%c%c%c%c%c\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
		printf("buf was unequal to MAKSTRIPE_CLONE_RESP.\n");
		return -1;
	}
	printf("Ready to clone from Makstripe buffer to card.\n");
	printf("Please swipe blank card\n");

	/* Read the response and make sure it matches MAKSTRIPE_CLONE_STS_OK */
	c = serial_read(fd, buf, strlen(MAKSTRIPE_CLONE_STS_OK));
	if (memcmp(buf, MAKSTRIPE_CLONE_STS_OK, strlen(MAKSTRIPE_CLONE_STS_OK)) != 0) {
		printf("We expect a string of length: %i\n", strlen(MAKSTRIPE_CLONE_STS_OK));
		printf("buf was unequal to MAKSTRIPE_CLONE_STS_OK: %s\n", MAKSTRIPE_CLONE_STS_OK);
		printf("%c%c%c%c%c\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
		return -1;
	}

	printf("Clone succesfull: %i\n", c);
	return c;
}

/* The MAKStripe is a bit of a pain and has failures cloning often. Wrap it.*/
int
mak_successful_clone(int fd)
{
	int r;
	do {
		r = mak_clone(fd);
	} while (r != 0);
	return r;
}
