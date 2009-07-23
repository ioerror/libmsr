/* This is an attempt at reversing the MAKStripe usb device magic numbers. */
/* It appears to be a very similar protocol to the MSR-206. */
/* The device may have as many as 20 possible card read buffers internally? */
/* We have confirmed only a single read buffer; it appears to be clobbered */
/* by all subsequent reads. */

/* Populate the buffer in the MAKStripe from the reader head. */
/* Returns populated data from the buffer in the MAKStripe to the host computer. */
#define MAKSTRIPE_READ 0x52 /* Single hex byte for ascii letter 'R' */
#define MAKSTRIPE_READY_TO_READ_RESPONSE "Ready"
/* Swipe a card here and wait for data. */
/* Sample data follows and ends with the status response. */
#define MAKSTRIPE_READ_STS_OK "RD=OK"
#define MAKSTRIPE_READ_STS_ERR /* UNKNOWN */

/* Populate the buffer in the MAKStripe from the host computer. */
/* Data is packed in an unknown format as of yet. */
#define MAKSTRIPE_POPULATE_BUF 'X' /* X<num of bytes><0x7> */
#define MAKSTRIPE_POPULATE_BUF_RESP "WB" /* Acknowledge that MAKStripe is ready for data. */
/* Now write out bytes to <fd> */
#define MAKSTRIPE_POPULATE_BUF_OK "WB=OK" /* Literal "WB=OK" */
#define MAKSTRIPE_POPULATE_BUF_ERR /* UNKNOWN */

/* Undefined as of yet but appears to be a valid command byte. */
#define MAKSTRIPE_WRITE_BUF 0x58 /* 'W' */
#define MAKSTRIPE_WRITE_BUF_STS_OK "WB" /* 574220 */
#define MAKSTRIPE_WRITE_BUF_STS_ERR /* UNKNOWN */

/* It's possible to swipe a reference card and then issue a clone command. */
/* It appears to buffer the reference card in the device and then write it */
/* to the next. MAKSTRIPE_CLONE esentially copies the buffer onto the card. */
/* Cloning steps: Issue MAKSTRIPE_READ and follow it with MAKSTRIPE_CLONE */
#define MAKSTRIPE_CLONE 0x43 /* ascii "C" */
#define MAKSTRIPE_CLONE_READY_TO_CLONE_RESPONSE "CP"
#define MAKSTRIPE_CLONE_STS_OK "CP=OK" /* Really pedobear? Party van is on the way! */
#define MAKSTRIPE_CLONE_STS_ERR /* UNKNOWN */
