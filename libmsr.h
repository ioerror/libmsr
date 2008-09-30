#define MSR_ESC 0x1B
#define MSR_ISO_TRACK_1_LEADING_ZEROS 0x3d
#define MSR_ISO_TRACK_2_LEADING_ZEROS 0x16
#define MSR_ISO_TRACK_3_LEADING_ZEROS 0x3d

#define MSR_STARTDELIM 0x73

#define MSR_ENDDELIM1 0x3F
#define MSR_ENDDELIM2 0x3F

#define MSR_RESET 0x61
#define MSR_RESET_RESPONSE  "" /* No response */

#define MSR_COMMTEST 0x65
#define MSR_COMMTEST_RESPONSE_SUCCESS 'y'

#define MSR_ISO_READ 0x72
#define MSR_ISO_READ_RESPONSE /* [data block]MSR_ESC[status byte] */

#define MSR_ISO_WRITE 0x77
#define MSR_ISO_WRITE_RESPONSE MSR_ESC /* MSR_ESC[status byte] */

#define MSR_READRAW 0x6D
#define MSR_READRAW_RESPONSE [raw data block]MSR_ESC[StatusByte]
#define MSR_WRITERAW 0x6E
#define MSR_WRITERAW_RESPONSE MSR_ESC[StatusByte] 

#define MSR_ALL_LED_OFF 0x81
#define MSR_ALL_LED_OFF_RESPONSE "" /* None */

#define MSR_ALL_LED_ON 0x82
#define MSR_ALL_LED_ON_RESPONSE "" /* None */

#define MSR_GREEN_LED_ON 0x83
#define MSR_GREEN_LED_ON_RESPONSE "" /* None */

#define MSR_YELLOW_LED_ON 0x84
#define MSR_YELLOW_LED_ON_RESPONSE "" /* None */

#define MSR_RED_LED_ON 0x85
#define MSR_RED_LED_ON_RESPONSE "" /* None */

/* You must either swipe a card or reset the MSR206 unit after this test */
#define MSR_SENSOR_TEST 0x86
#define MSR_SENSOR_TEST_RESPONSE_SUCCESS 0x30

/* This causes the MSR206 to perform a self test of its RAM */
#define MSR_RAM_TEST 0x87
#define MSR_RAM_TEST_RESPONSE_SUCCESS 0x30
#define MSR_RAM_TEST_RESPONSE_FAILURE 0x41

/* This command will return a string with the current hardware device model */
#define MSR_GET_DEVICE_MODEL MSR_ESC0x74
#define MSR_GET_DEVICE_MODEL_RESPONSE_MSR_206_1 MSR_ESC[model]S // Track 2
#define MSR_GET_DEVICE_MODEL_RESPONSE_MSR_206_2 MSR_ESC[model]S // Track 2 and 3
#define MSR_GET_DEVICE_MODEL_RESPONSE_MSR_206_3 MSR_ESC[model]S // Track 1, 2 and 3
#define MSR_GET_DEVICE_MODEL_RESPONSE_MSR_206_5 MSR_ESC[model]S // Track 1 and 2

/*
 * fill this in later
#define MSR_SET_LEADING_ZERO MSR_ESC 0x7a[leading zero of track 1 &3][leading zero of track 2]

*/

#define MSR_CHECK_LEADING_ZERO MSR_ESC0x6c
#define MSR_CHECK_LEADING_ZERO_RESPONSE MSR_ESC[00~ff][00~ff]

#define MSR_TRACK_1_SELECT_BYTE_ONLY 00000000
#define MSR_TRACK_2_SELECT_BYTE_ONLY 00000010
#define MSR_TRACK_3_SELECT_BYTE_ONLY 00000100

#define MSR_TRACK_1_AND_2_SELECT_BYTE_ONLY 00000011
#define MSR_TRACK_1_AND_3_SELECT_BYTE_ONLY 00000101
#define MSR_TRACK_2_AND_3_SELECT_BYTE_ONLY 00000110
#define MSR_TRACK_1_AND_2_AND_3_SELECT_BYTE_ONLY 00000111

#define MSR_ERASE_CARD MSR_ESC0x63[select byte]
#define MSR_ERASE_CARD_RESPONSE_SUCCESS MSR_ESC[0x0][0x1b][0x30]
#define MSR_ERASE_CARD_RESPONSE_FAILURE MSR_ESC[0xa][0x1b][0x41]

/* This command selects the density of the second track in BPI (210 or 75) */
#define MSR_SELECT_TRACK_2_BPI_210 MSR_ESC0xb0xd2
#define MSR_SELECT_TRACK_2_BPI_210_RESPONSE_SUCCESS MSR_ESC0x0[0x1b][0x30]
#define MSR_SELECT_TRACK_2_BPI_210_RESPONSE_FAILURE MSR_ESC0x0[0x1b][0x41]
#define MSR_SELECT_TRACK_2_BPI_75 MSR_ESC0xb0x4b
#define MSR_SELECT_TRACK_2_BPI_75_RESPONSE_SUCCESS MSR_ESC0xb0x4b
#define MSR_SELECT_TRACK_2_BPI_75_RESPONSE_FAILURE MSR_ESC0xb0x4b

