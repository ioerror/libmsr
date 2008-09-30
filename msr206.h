
#ifdef _MSR206_H_

/* ESC is frequently used as a start delimiter character */

#define MSR_ESC			0x1B	/* Escape character */

/* ASCII file separator character is used to separate track data */

#define MSR_FS			0x1C	/* File separator */

#define MSR_STS_OK		0x30	/* Ok */
#define MSR_STS_ERR		0x41	/* General error */

/* Read/write commands */


#define MSR_CMD_READ		0x72	/* Formatted read */
#define MSR_CMD_WRITE		0x77	/* Formatted write */
#define MSR_CMD_RAW_READ	0x6D	/* Raw read */
#define MSR_CMD_RAW_WRITE	0x6E	/* Raw write */

/* Status byte values from read/write commands */

#define MSR_STS_RW_ERR		0x31	/* Read/write error */
#define MSR_STS_RW_CMDFMT_ERR	0x32	/* Command format error */
#define MSR_STS_RW_CMDBAD_ERR	0x34	/* Invalid command */
#define MSR_STS_RW_SWIPEBAD_ERR	0x39	/* Invalud card swipe in write mode */

/* Read/write start and end delimitesr. */

#define MSR_RW_START		0x73	/* 's' */
#define MSR_RW_END		0x3F	/* '?' */

/*
 * Serial port communications test
 * If serial communications are working properly, the device
 * should respond with a 'y' command.
 */

#define MSR_CMD_DIAG_COMM	0x65	/* Communications test */
#define MSR_STS_COMM_OK		MSR_STS_OK

/*
 * Sensor diagnostic command. Will respond with MSR_STS_OK once
 * a card swipe is detected. Can be interrupted by a reset.
 */

#define MSR_CMD_DIAG_SENSOR	0x86	/* Card sensor test */
#define MSR_STS_SENSOR_OK	MSR_STS_OK

/*
 * RAM diagnostic command. Will return MSR_STS_OK if RAM checks
 * good, otherwise MSR_STS_ERR.
 *

#define MSR_CMD_DIAG_RAM	0x87	/* RAM test */
#define MSR_STS_RAM_OK		MSR_STS_OK
#define MSR_STS_RAM_ERR		MSR_STS_ERR

/*
 * Set leading zero count. Responds with MSR_STS_OK if values
 * set ok, otherwise MSR_STS_ERR
 */

#define MSR_CMD_SLZ		0x7A	/* Set leading zeros */
#define MSR_STS_SLZ_OK		MSR_STS_OK
#define MSR_STS_SLZ_OK		MSR_STS_ERR

/*
 * Get leading zero count. Returns leading zero counts for
 * track 1/3 and 2.
 */

#define MSR_CMD_CLZ		0x6C	/* Check leading zeros */

typedef struct msr_lz {
	uint8_t		msr_lz_tk1_3;
	uint8_t		msr_lz_tk2;
} msr_lz_t;

/*
 * Erase card tracks. Returns MSR_STS_OK on success or
 * MSR_STS_ERR.
 */

#define MSR_CMD_ERASE		0x63	/* Erase card tracks */
#define MSR_STS_ERASE_OK	MSR_STS_OK
#define MSR_STS_ERASE_ERR	MSR_STS_ERR

#define MSR_ERASE_TK1		0x00
#define MSR_ERASE_TK2		0x01
#define MSR_ERASE_TK3		0x02
#define MSR_ERASE_TK1		0x03
#define MSR_ERASE_TK1_TK2	0x04
#define MSR_ERASE_TK1_TK3	0x05
#define MSR_ERASE_TK2_TK3	0x06
#define MSR_ERASE_ALL		0x07

/*
 * Set bits per inch. Returns MSR_STS_OK on success or
 * MSR_STS_ERR.
 */

#define MSR_CMD_SETBPI		0x62	/* Set bits per inch */
#define MSR_STS_BPI_OK		MSR_STS_OK
#define MSR_STS_BPI_ERR		MSR_STS_ERR

/*
 * Get device model number. Returns a value indicating a model
 * number, plus an 'S'.
 */

#define MSR_CMD_MODEL		0x74	/* Read model */
#define MSR_STS_MODEL_OK	0x53

#define MSR_MODEL_MSR206_1	0x1
#define MSR_MODEL_MSR206_2	0x2
#define MSR_MODEL_MSR206_3	0x3
#define MSR_MODEL_MSR206_5	0x5

/*
 * Get firmware revision. Response is a string in
 * the form of "REV?X.XX" where X.XX is the firmware
 * rev, and ? can be:
 *
 * MSR206: '0'
 * MSR206HC: 'H'
 * MSR206HC: 'L'
 */

#define MSR_CMD_FWREV		0x76	/* Read firmware revision */
#define MSR_FWREV_FMT		"REV?X.XX"

/*
 * Set bits per character. Returns MSR_STS_OK on success, accompanied
 * by resulting per-track BPC settings.

#define MSR_CMD_SETBPC		0x6F	/* Set bits per character */
#defien MSR_STS_BPC_OK		MSR_STS_OK
#defien MSR_STS_BPC_ERR		MSR_STS_ERR

typedef struct msr_bpc {
	uint8_t		msr_bpctk1;
	uint8_t		msr_bpctk2;
	uint8_t		msr_bpctk3;
} msr_bpc_t;

/*
 * Set coercivity high or low. Returns MSR_STS_OK on success.
 */

#define MSR_CMD_SETCO_HI	0x78	/* Set coercivity high */
#define MSR_CMD_SETCO_LO	0x79	/* Set coercivity low */
#defien MSR_STS_CO_OK		MSR_STS_OK
#defien MSR_STS_CO_ERR		MSR_STS_ERR

/*
 * Get coercivity. Returns 'H' for high coercivity, 'L' for low.
 */

#define MSR_CMD_GETGO		0x64	/* Read coercivity setting */
#define MSR_CO_HI		0x48
#define MSR_CO_LO		0x4C

/* The following commands have no response codes */

#define MSR_CMD_RESET		0x61	/* Reset device */
#define MSR_CMD_LED_OFF		0x81	/* All LEDs off */
#define MSR_CMD_LED_ON		0x82	/* All LEDs on */
#define MSR_CMD_LED_GRN_ON	0x83	/* Green LED on */
#define MSR_CMD_LED_YLW_ON	0x84	/* Yellow LED on */
#define MSR_CMD_LED_RED_ON	0x85	/* Red LED on */

#endif /* _MSR206_H_ */
