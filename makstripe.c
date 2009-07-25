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
	char buf[strlen(MAK_RESET_RESP)];
	memset(buf, 0, strlen(MAK_RESET_RESP));
	/* We're expecting something like: MSUSB CI.270209 */
	/*
	f64f3740 177877074 S Bi:5:017:1 -115 64 <
	f64f3740 177878048 S Bi:5:017:1 -115 64 <
	f688ce40 177882078 S Bo:5:017:1 -115 1 = 3f
	f688ce40 177883039 C Bo:5:017:1 0 1 >
	f64f3740 177887027 C Bi:5:017:1 0 15 = 4d535553 42204349 2e323730 323039
	f64f3740 177887039 S Bi:5:017:1 -115 64 <
	f64f3740 185363008 C Bi:5:017:1 -2 0
	*/
	printf("Sending reset command: %c\n", MAK_RESET_CMD);
	serial_write(fd,MAK_RESET_CMD, sizeof(MAK_RESET_CMD));

	printf("We expect: %s\n", MAK_RESET_RESP);
	printf("We get...:\n");
	for (i = 0; i < 10; i++) {
		serial_read(fd, buf, 2);
		printf("%02x %02x \n", buf[0], buf[1]);
	}

	printf("Reset status: %d", r);
	return 0;
}

int
mak_read(int fd, uint8_t tracks)
{
	int r;
	int i;
	char buf[5];
	uint16_t sample_count;
	printf("Attempting to perform a read...\n");

	r = mak_cmd(fd, MAKSTRIPE_READ_CMD, tracks);

	/* Ready response is needed here, check for this. */
	serial_read(fd, buf, strlen(MAKSTRIPE_READ_RESP));
	printf("We got: %s\n", buf);
	printf("We expected: %s\n", MAKSTRIPE_READ_RESP);
	if (!memcmp(buf, MAKSTRIPE_READ_RESP, strlen(MAKSTRIPE_READ_RESP)))
		printf("Doh!\n");
		return -1;
	printf("Please swipe a card for read...\n");

	/* 'RD '<16bits of length data><data samples> */
	serial_read(fd, buf, 3);
	printf("We got: %s\n", buf);
	if (!memcmp(buf, MAKSTRIPE_READ_BUF_PREFIX, strlen(MAKSTRIPE_READ_BUF_PREFIX)));
		return -1;
	serial_read(fd, &sample_count, 2);
	printf("Sample count appears to be: %d\n", sample_count);

	/* XXX: We currently clobber the data. We don't return it. */
	for (i = 0; i < sample_count; i++) {
		printf("%02x %02x \n", buf[0], buf[1]);
		serial_read(fd, buf, 2);
	}

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
