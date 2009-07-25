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

/* Remember that the MAKStripe desires MAK_BAUD for serial io. */

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

int
mak_read(int fd, uint8_t tracks)
{
	int r;
	int i;
	char buf[5];
	char sample_tmp[2];
	uint16_t sample_count;
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

	/* XXX: We currently clobber the data. We don't return it. */
	for (i = 0; i < sample_count; i++) {
		printf("%d %02x %02x\n", i, sample_tmp[0], sample_tmp[1]);
		serial_read(fd, sample_tmp, 2);
	}

	printf("In theory, we have dumped the full sample data now...");
	/* Lets ensure that the sample read went correctly! */
	serial_read(fd, buf, 5);
	printf("Sample read returned status: %s\n", buf);

	if (!memcmp(buf, MAKSTRIPE_READ_STS_OK, sizeof(MAKSTRIPE_READ_STS_OK)))
		return -1;

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
	char buf[5];

	c = mak_cmd(fd, MAKSTRIPE_CLONE_CMD, 0);

	if (c != 0) {
		printf("Clone command failure.\n");
		return -1;
	}

	/* Read the response and make sure it matches MAKSTRIPE_CLONE_RESP */
	serial_read(fd, buf, sizeof(MAKSTRIPE_CLONE_RESP));
	if (!memcmp(buf, MAKSTRIPE_CLONE_RESP, sizeof(MAKSTRIPE_CLONE_RESP)))
		return -1;

	printf("Ready to clone from Makstripe buffer to card.\n");
	printf("Please swipe blank card\n");

	/* Read the response and make sure it matches MAKSTRIPE_CLONE_STS_OK */
	serial_read(fd, buf, sizeof(MAKSTRIPE_CLONE_STS_OK));
	if (!memcmp(buf, MAKSTRIPE_CLONE_STS_OK, sizeof(MAKSTRIPE_CLONE_STS_OK)))
		return -1;

	printf("Clone succesfull.\n");
	return c;
}
