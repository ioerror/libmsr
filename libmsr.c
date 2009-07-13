#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "libmsr.h"

int
msr_dumpbits (uint8_t * buf, int len)
{
	int		bytes, i;

	for (bytes = 0; bytes < len; bytes++) {

		/*
		 * Note: we want to display the bits in the order in
		 * which they're read off the card, which means we
		 * have to decode each byte from most significant bit
		 * to least significant bit.
		 */

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
msr_getbit (uint8_t * buf, uint8_t len, int bit)
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
msr_setbit (uint8_t * buf, uint8_t len, int bit, int val)
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


int
msr_decode(uint8_t * inbuf, uint8_t inlen,
    uint8_t * outbuf, uint8_t * outlen, int bpc)
{
	uint8_t * b;
	uint8_t len;
	int ch = 0;
	char byte = 0;
	int i, x;

	len = inlen;
	b = inbuf;
	x = 0;

	for (i = 0; i < len * 8; i++) {
		byte |= msr_getbit (b, len, i) << ch;
		if (ch == (bpc - 1)) {
			/* Strip the parity bit */
			byte &= ~(1 << ch);
			if (bpc < 7)
				byte |= 0x30;
			else {
				if (byte < 0x20)
					byte |= 0x20;
				else {
					byte |= 0x40;
					byte -= 0x20;
				}
			}

			outbuf[x] = byte;
			x++;
			/* Don't overflow output buffer */
			if (x == *outlen)
				break;
#ifdef MSR_DEBUG
			printf ("%c", byte);
#endif
			ch = 0;
			byte = 0;
		} else
			ch++;
	}

#ifdef MSR_DEBUG
	printf ("\n");
#endif

	/* Output buffer was too small. */
	if (x == *outlen)
		return (-1);
	*outlen = x;

	return (0);
}

/* Some cards require a swipe in the opposite direction of the reader. */
/* We can get the expected bit stream by reversing the data in place. */
int
msr_reverse_tracks (msr_tracks_t * tracks)
{
	int i, status;
	for (i = 0; i < MSR_MAX_TRACKS; i++) {
		status = msr_reverse_track(i, tracks);
		if (status != 0) {
			printf("Unable to reverse track: %i\n", i);
			status = -1;
			break;
		}
	}
	return status;
}

/* We want to take a track and reverse the order of each byte. */
/* Additionally, we want to flip each byte. */
int
msr_reverse_track (int track_number, msr_tracks_t * tracks)
{
	int i;
	int status = 0;
	int bytes_to_shuffle;
	char first_byte;
	char last_byte;
	unsigned char * head;
	unsigned char * tail;

	/* First we need to know the size of the track */
	bytes_to_shuffle = tracks->msr_tracks[track_number].msr_tk_len;

	/* Then we need to read each byte from a track */
	for (i=0; i <= bytes_to_shuffle / 2; i++) {
		head = &tracks->msr_tracks[track_number].msr_tk_data[i];
		tail = &tracks->msr_tracks[track_number].msr_tk_data[(bytes_to_shuffle -1) -i];
		/* Reverse the full track byte order */
		first_byte = msr_reverse_byte(*head);
		last_byte = msr_reverse_byte(*tail);
		/* Reverse the byte order */
		*head = last_byte;
		*tail = first_byte;
	}
	return status;
}

/* Reverse a byte. */
const unsigned char
msr_reverse_byte(const unsigned char byte)
{
	return
	((byte & 1<<7) >> 7) |
	((byte & 1<<6) >> 5) |
	((byte & 1<<5) >> 3) |
	((byte & 1<<4) >> 1) |
	((byte & 1<<3) << 1) |
	((byte & 1<<2) << 3) |
	((byte & 1<<1) << 5) |
	((byte & 1<<0) << 7);
}
