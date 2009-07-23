/* This is an attempt at reversing the MAKStripe usb device magic numbers. */
/* It appears to be a very similar protocol to the MSR-206. */

#define MAKSTRIPE_READ 0x52 /* Single hex byte for ascii letter 'R' */
#define MAKSTRIPE_READ_STS_OK "Ready"  /*  52656164 79 */
#define MAKSTRIPE_READ_STS_ERR /* UNKNOWN */

#define MAKSTRIPE_POPULATE_BUF 'X' /**/
#define MAKSTRIPE_POPULATE_BUF_RESP "WB=OK" /* Literal "WB=OK" */
#define MAKSTRIPE_POPULATE_BUF_OK "WB" /* Literal "WB " */
#define MAKSTRIPE_POPULATE_BUF_ERR /* UNKNOWN */

#define MAKSTRIPE_WRITE 0x58 /* 'W' */
#define MAKSTRIPE_WRITE_STS_OK "WB" /* 574220 */
#define MAKSTRIPE_WRITE_STS_ERR /* UNKNOWN */

/* It's possible to swipe a reference card and then issue a clone command. */
/* It appears to be buffer the reference card in the device and then write it */
/* to the second card. */
#define MAKSTRIPE_CLONE 0x43 /* ascii "C" */
#define MAKSTRIPE_CLONE_STS_OK "CP=OK" /* Really pedobear? Party van is on the way! */
#define MAKSTRIPE_CLONE_STS_ERR /* UNKNOWN */
