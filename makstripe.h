/* This is an attempt at reversing the MAKStripe usb device magic numbers. */
/* It appears to be a very similar protocol to the MSR-206. */
/* The device may have as many as 20 possible card read buffers internally? */
/* We have confirmed only a single read buffer; it appears to be clobbered */
/* by all subsequent reads. */
/* Please see the README.MAKStripe file for more information. */

/*
 * Read (populate buffer from card, and show us the data):
 * Send: R<0x7>
 * Response: Ready
 * <swipe card>
 * Response: RD<0xXX><0xYY><0x20><data>RD=OK
 */

/*
 * Populate buffer from host:
 * Send: X<number of bytes><0x7>
 * Response: WB<space>
 * Send: data samples
 * Response: WB=OK
 */

/*
 * Populate buffer from card (read, but don't show us the data)
 * Send: W
 * Response: RA
 * <swipe card>
 * Response: RA=OK
 */

/*
 * Copy buffer (copy data to card card)
 * Send: C<0x7>
 * Response: CP<space>
 * <swipe card>
 * Response: CP=OK
 */

/* This is the basic way to send a command */
/* This appears to be the way that R/W/S/C operate. */
typedef struct mak_cmd {
	uint8_t	mak_cmd;
	uint8_t	mak_track_mask;
} mak_generic_cmd_t;

typedef struct mak_cmd_load_buf {
	uint8_t	mak_cmd;
	uint16_t	mak_len;
} mak_load_buf_t;

/* This is for the erase/eRase commands. */
typedef struct mak_cmd_erase {
	uint8_t	mak_cmd;
	uint8_t	mak_track_mask;
	uint8_t	mak_wtf;
} mak_cmd_erase_t;

/* This is the byte sent as the suffix for all commands. */
#define MAK_ESC 0x04 /* The bits formerly known as <EOT> */

/* This command is possibly a command that resets the MAKStripe. */
/* It appears that after sending this command, the device prints some data. */
/* At first, we thought that this might be the firmware query command. */
/* However, it appears that this is used to cancel operations in progress */
/* additionally, it's used at other times. */
/* It is likely an unintended consequence that this produces a firmware */
/* version string. It probably does this because this command resets the */
/* device and it prints a boot loader or something to its serial port. */
#define MAK_FIRMWARE_QUERY_CMD '?' /* ?<MAK_ESC>*/
#define MAK_FIRMWARE_QUERY_RESP /* The response is the firmware information. */
#define MAK_FIRMWARE_QUERY_STS_OK /* UNKNOWN */
#define MAK_FIRMWARE_QUERY_STS_ERR /* UNKNOWN */

/* Populate the buffer in the MAKStripe from the reader head. */
/* Returns populated data from the buffer in the MAKStripe to the host computer. */
#define MAKSTRIPE_READ_CMD 'R' /* R<MAK_ESC> */
#define MAKSTRIPE_READ_RESP "Ready" /* Sing it: "One of these things is not like the others..." */
/* Swipe a card here and wait for data. */
/* Sample data follows and ends with the status response. */
/* Sample data format is as follows: 'RD '<16bits of length data><data samples> */
#define MAKSTRIPE_READ_BUF_PREFIX "RD "
#define MAKSTRIPE_READ_STS_OK "RD=OK"
#define MAKSTRIPE_READ_STS_ERR /* UNKNOWN */

/* Populate the buffer in the MAKStripe from the host computer. */
/* Data is packed in an unknown format as of yet. */
#define MAKSTRIPE_POPULATE_BUF_CMD 'X' /* X<num of bytes><MAK_ESC> */
#define MAKSTRIPE_POPULATE_BUF_RESP "WB " /* Acknowledge that MAKStripe is ready for data. */
/* Now write out bytes to <fd> */
#define MAKSTRIPE_POPULATE_BUF_OK "WB=OK" /* Literal "WB=OK" */
#define MAKSTRIPE_POPULATE_BUF_ERR /* UNKNOWN */

/* Show what's in the device buffer. */
#define MAKSTRIPE_SHOW_BUFFER_CMD 'S' /*S<MAK_ESC>*/
#define MAKSTRIPE_SHOW_BUFFER_RESP /* The response is the buffered data. */
#define MAKSTRIPE_SHOW_BUFFER_STS_OK "RB=1 OK"
#define MAKSTRIPE_SHOW_BUFFER_STS_ERR /* UNKNOWN */

/* Undefined as of yet but appears to be a valid command byte. */
#define MAKSTRIPE_WRITE_BUF_CMD 'W' /* W<MAK_ESC> */
#define MAKSTRIPE_WRITE_BUF_RESP /* UNKNOWN */
#define MAKSTRIPE_WRITE_BUF_STS_OK "WB "
#define MAKSTRIPE_WRITE_BUF_STS_ERR /* UNKNOWN */

/* It's possible to swipe a reference card and then issue a clone command. */
/* It appears to buffer the reference card in the device and then write it */
/* to the next. MAKSTRIPE_CLONE esentially copies the buffer onto the card. */
/* Cloning steps: Issue MAKSTRIPE_READ and follow it with MAKSTRIPE_CLONE */
#define MAKSTRIPE_CLONE_CMD 'C' /* W<MAK_ESC> */
#define MAKSTRIPE_CLONE_RESP "CP "
#define MAKSTRIPE_CLONE_STS_OK "CP=OK" /* Really pedobear? Party van is on the way! */
#define MAKSTRIPE_CLONE_STS_ERR /* UNKNOWN */

/* These are the generic ways that we can expect to commonly discuss a track */
#define MAKSTRIPE_TK1	0x01
#define MAKSTRIPE_TK2	0x02
#define MAKSTRIPE_TK3	0x04
#define MAKSTRIPE_TK_ALL (MAKSTRIPE_TK1 | MAKSTRIPE_TK2 | MAKSTRIPE_TK3)

/*
* These are the magic bytes for the format command
* The format command seems to be an 'F' followed by a single byte
* track mask, followed by " d" (space, lower case d). It's unclear
* what the " d" means.
*/

#define MAKSTRIPE_FMT_CMD	'F' /* F<MAKSTRIPE_FMT_TK1>" d" */
#define MAKSTRIPE_FMT_RESP	"FM "
#define MAKSTRIPE_FMT_OK	"FM=OK"
#define MAKSTRIPE_FMT_ERR	/* UNKNOWN */
#define MAKSTRIPE_FMT_TK1	MAKSTRIPE_TK1
#define MAKSTRIPE_FMT_TK2	MAKSTRIPE_TK2
#define MAKSTRIPE_FMT_TK3	MAKSTRIPE_TK3
#define MAKSTRIPE_FMT_TK1_TK2	MAKSTRIPE_TK1 | MAKSTRIPE_TK2 /* Should be: 0x03 */
#define MAKSTRIPE_FMT_TK1_TK3	MAKSTRIPE_TK1 | MAKSTRIPE_TK3 /* Should be: 0x05 */
#define MAKSTRIPE_FMT_TK2_TK3	MAKSTRIPE_TK2 | MAKSTRIPE_TK3 /* Should be: 0x06 */
#define MAKSTRIPE_FMT_ALL	MAKSTRIPE_TK_ALL /*  etc: 0x07 */

/* These are the magic bytes for the Erase command */
/* These are the low flux bit erase commands */
/* "Erase selected tracks in FLUX 0 direction." */
#define MAKSTRIPE_ErASE_CMD	'E' /* E<MAKSTRIPE_FMT_TK1><MAK_ESC> XXX: Confirm with usb dump */
#define MAKSTRIPE_ErASE_RESP	"Er "
#define MAKSTRIPE_ErASE_OK	"Er=OK"
#define MAKSTRIPE_ErASE_ERR	/* UNKNOWN */
#define MAKSTRIPE_ErASE_TK1	MAKSTRIPE_TK1
#define MAKSTRIPE_ErASE_TK2	MAKSTRIPE_TK2
#define MAKSTRIPE_ErASE_TK3	MAKSTRIPE_TK3
#define MAKSTRIPE_ErASE_TK1_TK2	MAKSTRIPE_TK1 | MAKSTRIPE_TK2 /* Should be: 0x03 */
#define MAKSTRIPE_ErASE_TK1_TK3	MAKSTRIPE_TK1 | MAKSTRIPE_TK3 /* Should be: 0x05 */
#define MAKSTRIPE_ErASE_TK2_TK3	MAKSTRIPE_TK2 | MAKSTRIPE_TK3 /* Should be: 0x06 */
#define MAKSTRIPE_ErASE_ALL	MAKSTRIPE_TK1 | MAKSTRIPE_TK2 | MAKSTRIPE_TK3 /*  etc: 0x07 */

/* These are the magic bytes for the eRase command */
/* These are the high flux bit erase commands */
/* "Erase selected tracks in FLUX 1 direction." */
#define MAKSTRIPE_eRASE_CMD	'e' /* e<MAKSTRIPE_FMT_TK1><MAK_ESC> XXX: Confirm with usb dump */
#define MAKSTRIPE_eRASE_RESP	"eR "
#define MAKSTRIPE_eRASE_OK	"eR=OK"
#define MAKSTRIPE_eRASE_ERR	/* UNKNOWN */
#define MAKSTRIPE_eRASE_TK1	MAKSTRIPE_TK1
#define MAKSTRIPE_eRASE_TK2	MAKSTRIPE_TK2
#define MAKSTRIPE_eRASE_TK3	MAKSTRIPE_TK3
#define MAKSTRIPE_eRASE_TK1_TK2	MAKSTRIPE_TK1 | MAKSTRIPE_TK2 /* Should be: 0x03 */
#define MAKSTRIPE_eRASE_TK1_TK3	MAKSTRIPE_TK1 | MAKSTRIPE_TK3 /* Should be: 0x05 */
#define MAKSTRIPE_eRASE_TK2_TK3	MAKSTRIPE_TK2 | MAKSTRIPE_TK3 /* Should be: 0x06 */
#define MAKSTRIPE_eRASE_ALL	MAKSTRIPE_TK1 | MAKSTRIPE_TK2 | MAKSTRIPE_TK3 /*  etc: 0x07 */
