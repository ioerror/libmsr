/* This is an attempt at reversing the MAKStripe usb device magic numbers. */
/* It appears to be a very similar protocol to the MSR-206. */
/* The device may have as many as 20 possible card read buffers internally? */
/* We have confirmed only a single read buffer; it appears to be clobbered */
/* by all subsequent reads. */
/* Please see the README.MAKStripe file for more information. */

#define MAK_ESC 'A'
#define MAK_FIRMWARE_QUERY_CMD '?'

/* Populate the buffer in the MAKStripe from the reader head. */
/* Returns populated data from the buffer in the MAKStripe to the host computer. */
#define MAKSTRIPE_READ_CMD "R" /* 0x52 */
#define MAKSTRIPE_READ_RESP "Ready"
/* Swipe a card here and wait for data. */
/* Sample data follows and ends with the status response. */
#define MAKSTRIPE_READ_STS_OK "RD=OK"
#define MAKSTRIPE_READ_STS_ERR /* UNKNOWN */

/* Populate the buffer in the MAKStripe from the host computer. */
/* Data is packed in an unknown format as of yet. */
#define MAKSTRIPE_POPULATE_BUF_CMD 'X' /* X<num of bytes><0x7> */
#define MAKSTRIPE_POPULATE_BUF_RESP "WB " /* Acknowledge that MAKStripe is ready for data. */
/* Now write out bytes to <fd> */
#define MAKSTRIPE_POPULATE_BUF_OK "WB=OK" /* Literal "WB=OK" */
#define MAKSTRIPE_POPULATE_BUF_ERR /* UNKNOWN */

/* Undefined as of yet but appears to be a valid command byte. */
#define MAKSTRIPE_WRITE_BUF_CMD "W" /* 0x58 */
#define MAKSTRIPE_WRITE_BUF_RESP /* UNKNOWN */
#define MAKSTRIPE_WRITE_BUF_STS_OK "WB "
#define MAKSTRIPE_WRITE_BUF_STS_ERR /* UNKNOWN */

/* It's possible to swipe a reference card and then issue a clone command. */
/* It appears to buffer the reference card in the device and then write it */
/* to the next. MAKSTRIPE_CLONE esentially copies the buffer onto the card. */
/* Cloning steps: Issue MAKSTRIPE_READ and follow it with MAKSTRIPE_CLONE */
#define MAKSTRIPE_CLONE_CMD "C" /* 0x43 */
#define MAKSTRIPE_CLONE_RESP "CP "
#define MAKSTRIPE_CLONE_STS_OK "CP=OK" /* Really pedobear? Party van is on the way! */
#define MAKSTRIPE_CLONE_STS_ERR /* UNKNOWN */

/* These are the generic ways that we can expect to commonly discuss a track */
#define MAKSTRIPE_TK1	0x01
#define MAKSTRIPE_TK2	0x02
#define MAKSTRIPE_TK3	0x04

/* These are the magic bytes for the format command */
#define MAKSTRIPE_FMT_CMD	"F" /* Eg: MAKSTRIPE_FMT_CMDMAKSTRIPE_FMT_TK1 */
#define MAKSTRIPE_FMT_RESP	"FM "
#define MAKSTRIPE_FMT_OK	"FM=OK"
#define MAKSTRIPE_FMT_ERR	/* UNKNOWN */
#define MAKSTRIPE_FMT_TK1	MAKSTRIPE_TK1
#define MAKSTRIPE_FMT_TK2	MAKSTRIPE_TK2
#define MAKSTRIPE_FMT_TK3	MAKSTRIPE_TK3
#define MAKSTRIPE_FMT_TK1_TK2	MAKSTRIPE_TK1 | MAKSTRIPE_TK2 /* Should be: 0x03 */
#define MAKSTRIPE_FMT_TK1_TK3	MAKSTRIPE_TK1 | MAKSTRIPE_TK3 /* Should be: 0x05 */
#define MAKSTRIPE_FMT_TK2_TK3	MAKSTRIPE_TK2 | MAKSTRIPE_TK3 /* Should be: 0x06 */
#define MAKSTRIPE_FMT_ALL	MAKSTRIPE_TK1 | MAKSTRIPE_TK2 | MAKSTRIPE_TK3 /*  etc: 0x07 */

/* These are the magic bytes for the Erase command */
/* These are the low flux bit erase commands */
#define MAKSTRIPE_ErASE_CMD	"E"
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
#define MAKSTRIPE_eRASE_CMD	"e"
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
