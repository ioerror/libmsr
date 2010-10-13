/*

Compile with:
gcc -o msr-bart-dumper msr-bart-dumper.c -L . -l msr

Fun fact of the yesteryears:
Bill Wattenburg discovered the BART card duplication security flaw in 1973!

Fun fact of the day:
It hasn't really changed much!

We only have a single track of custom formatted data.
We know that the card has 72 8 bit bytes of data.
We assume it is little endian (windows 2k x86 machines, eww) and bytes are again, 8 bits.
All bytes are 8 bit aligned.
We believe that the structure of the card is as follows (hex formatted) :

[00]
[E7] [11 byte field]
[E7] [11 byte field]
[5 bytes of 00]
[E7] [13 byte field]
[E7] [13 byte field]
[E7] [13 byte field]

Example:
E7 87 60 07 6C C5 5A 2B 1A 07 9D D8
00 00 00 00 00
E7 C4 04 00 FB DA E4 C5 48 40 07 FF 58 2F

#define PACKED __attribute__((__packed__))
struct
{
	uint8_t	start_delim; // '00' - This doesn't appear to be used... yet?
	uint8_t	card_rev_blob_00;    // 'e7' - This is card rev 231 when printed as a decimal
	uint8_t[11]	blobby_a; // '87 60 07 6C C5 5A 2B 1A 07 9D D8'
				  // '135 96 7 108 197 90 43 26 7 157 216' - unknown dec values
				  // Unknown values? What are these bytes?
	uint8_t	card_rev_blob_01; // 'e7'
	uint8_t[11]	blobby_a_copy; // '87 60 07 6C C5 5A 2B 1A 07 9D D8'
	uint8_t[5]	little_blobby_zero_tables; '00 00 00 00 00'
	uint8_t	card_rev_blob_02; // 'e7'
	uint8_t[13]	bigger_blobby_b; // 'C4 04 00 FB DA E4 C5 48 40 07 FF 58 2F'
	uint8_t	card_rev_blob_03; // 'e7'
	uint8_t[13]	bigger_blobby_b_copy_00; // 'C4 04 00 FB DA E4 C5 48 40 07 FF 58 2F'
	uint8_t	card_rev_blob_04; // 'e7'
	uint8_t[13]	bigger_blobby_b_copy_01; // 'C4 04 00 FB DA E4 C5 48 40 07 FF 58 2F'
} bart_card; PACKED

In theory, without knowing anything else about the card, we can write a very simple
parsing engine like so:

if ( bart_card->start_delim == '0x0' )
{
	if (bart_card->card_rev_blob_00 == bart_card->card_rev_blob_01)
	{
		if (bart_card->blobby_a_copy == bart_card->blobby_a_copy)
		{
			if (*(uint32_t *)&bart_card->little_blobby_zero_tables[0] == 0x00)
			{
			if (bart_card->bigger_blobby_b == bart_card->bigger_blobby_b_copy_00 &&
			bart_card->bigger_blobby_b_copy_00 == bart_card->bigger_blobby_b_copy_01 )
			{ printf("This looks like the card is correctly read.\n");}
			}
		}
	}

}

*/

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
	msr_tracks_t tracks;
	int i;

	int track_number = 1;
	int number_of_bytes = 72;
	int data_index;

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

	/* Set the reader into Lo-Co mode */
	msr_set_lo_co (fd);

	/* Ram test */
	msr_ram_test (fd);

	/* Prepare the reader with a reset */
	msr_init (fd);

do {
	bzero ((char *)&tracks, sizeof(tracks));

	for (i = 0; i < MSR_MAX_TRACKS; i++)
		tracks.msr_tracks[i].msr_tk_len = MSR_MAX_TRACK_LEN;

	printf("Ready to do a raw read with custom formatted data. Please slide a card.\n");
	msr_raw_read (fd, &tracks);

	/* Count and ensure we have 72 bytes for the middle track. */
	if (tracks.msr_tracks[track_number].msr_tk_len == number_of_bytes) {
		printf("This card is the proper byte count: %d\n", number_of_bytes);
		/* We want to reverse the bytes because the card was read backwards. */
		printf("Flipping the track data into the correct direction.\n");
		msr_reverse_tracks(&tracks);
		/*  We now want to check each byte that we know about. */
		for (data_index = 0; data_index < number_of_bytes; data_index++) {
			if (data_index == 0x0 &&
			tracks.msr_tracks[track_number].msr_tk_data[data_index] != 0x0) {
				printf("Bad start byte.\n");
				break;
			}
			if (data_index == 0x1 &&
			tracks.msr_tracks[track_number].msr_tk_data[data_index] != 0xE7) {
				printf("Unable to find the second expected byte.\n");
				printf("Got byte: %x\n", tracks.msr_tracks[track_number].msr_tk_data[data_index]);
				break;
			}
		}
		if (data_index == number_of_bytes ) {
			printf("This card appears to validate.\n");
		}
	} else {
		printf("This card does not validate.\n");
	}
	/* dump the bytes as a hexdump */
	printf("Hex output:\n");
	msr_pretty_printer_hex(tracks);
	/* dump the bytes as a string */
	printf("String output:\n");
	msr_pretty_printer_string(tracks);
} while (1);
	/* We're finished */
	serial_close (fd);
	exit(0);
}
