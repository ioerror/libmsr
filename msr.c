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

/* Thanks Club Mate and h1kari! Toorcon 10 */

int
getstart (int fd)
{
	char b;
	int i, r;

	for (i = 0; i < 3; i++) {
		r = read (fd, &b, 1);
		if (b == MSR_STARTDELIM)
			break;
	}

	if (i == 3)
		return (-1);

	return (0);
}

int
comtest (int fd)
{
    int r;
    char b[256];
    char buf[256];

    bzero (buf, sizeof(buf));

    buf[0] = MSR_ESC;
    buf[1] = MSR_COMMTEST;

    r = write (fd, buf, 2); 

    usleep (100000);

    if (fd == -1)
        err(1, "Commtest write failed");
/*
    else
        printf("Commtest wrote: %d\n", r);
*/
    /* read the result */
    read (fd, &b, 4);

    if (b[0] == MSR_ESC && b[1] == MSR_COMMTEST_RESPONSE_SUCCESS )
    {
        printf("Communications test successful\n");
        return 0;
    } else {
        printf("Communcations test failure\n");
    }
    
    return 1;

}

int 
get_firmware_version (int fd)
{
    int r;
    char b[256];
    char buf[256];

    bzero (buf, sizeof(buf));

    buf[0] = MSR_ESC;
    buf[1] = 0x76;

    r = write (fd, buf, 2);

    usleep (100000);

    if (fd == -1)
        err(1, "Firmware write failed");
    /* else
        printf("Firmware version wrote: %d\n", r);
    */

    /* read the result "REV?X.XX" */
    read (fd, &b, 9);
    b[9] = '\0';

    printf("Firmware version results: %s\n", b);
    if (b[0] == MSR_ESC && b[1] == 'y')
    {
        printf("MSR_COMMTEST SUCCESSFUL\n");
        return 0;
    }

    return 1;

}

int get_device_model (int fd)
{
    int r;
    char b[4];
    char buf[2];

    bzero (buf, sizeof(buf));

    buf[0] = MSR_ESC;
    buf[1] = 0x74;

    r = write (fd, buf, 2);

    usleep (100000);

    if (fd == -1)
        err(1, "Device Model write failed");
    /*else
        printf("Device Model wrote: %d\n", r);
    */

    /* read the result as the value of X in "MSR206-X" */
    read (fd, &b, 3);
    b[4] = '\0';

    if (b[1] == '1' || '2' || '3' || '5')
    {
        printf("Device Model detected: MSR-206-%c\n", b[1]);
        return 0;
    }
    
    return 1;
}

int
flash_led (int fd, uint8_t led)
{

    int r;
    uint8_t buf[2];

    bzero (buf, sizeof(buf));

    buf[0] = MSR_ESC;
    buf[1] = led;

    r = write (fd, buf, 2);

    usleep (100000);

    if (fd == -1)
        err(1, "LED failure");

    /* No response, look at the lights Dr. Love */
    return 0;

}

int 
do_leet_led_dance (fd)
{
    int z = 0;
    while(z < 10){

        flash_led(fd, MSR_ALL_LED_ON);
        flash_led(fd, MSR_GREEN_LED_ON);
        flash_led(fd, MSR_YELLOW_LED_ON);
        flash_led(fd, MSR_RED_LED_ON);
        flash_led(fd, MSR_ALL_LED_OFF);
        usleep (100000);
        z++;
    }

    return 0;
}

int
gettrack (int fd, int t)
{
	char b;

	/* Start delimiter should be ESC <track number> */

	read (fd, &b, 1);
	if (b != MSR_ESC) {
		printf("NOT AN ESCAPE (%x)...\n", b);
		return (-1);
	} else {
        printf("ESCAPE SEQUENCE FOUND\n");
    }
	read (fd, &b, 1);
	if (b != t)
		{
		printf("NOT A TRACK (%x)...\n", b);
		return (-1);
		}

	printf("GETTING TRACK %d WITH DATA CAST AS UNSIGNED CHAR\n", t);
	while (1) {
		read (fd, &b, 1);
		if (b == MSR_ENDDELIM1)
			break;
		printf("%c", b);
	}

	/* Should be another escape here */
	if (b == MSR_ENDDELIM1) {
		printf("\nTRACK %d READ FINISHED\n", t);
		return (0);
	}
	return (-1);
}

int 
msr_sensor_test (fd)
{
    int r;
    char b[4];
    unsigned char buf[3];

    bzero (buf, sizeof(buf));
    buf[0] = MSR_ESC;
    buf[1] = MSR_SENSOR_TEST;
    
    r = write (fd, buf, 2);

    printf("Attempting to read a response for the next five seconds.\n");
    printf("Please slide a card and wait...\n");
    sleep (5);

    /* read the result "<esc>0(0x1b0x30)" if OK */
    read (fd, &b, 2);
    b[2] = '\0';

    printf("Sensor test results: %s\n", b);
    if ( b[0] == MSR_ESC && b[1] == MSR_SENSOR_TEST_RESPONSE_SUCCESS )
    {
        printf("Sensor test successfull\n");
        return 0;
    }

    printf("It appears that the sensor did not sense a magnetic card.\n");
    return 1;

}

int
msr_reset (fd)
{
    int r;
    char    buf[2];

    bzero (buf, sizeof(buf));

    buf[0] = MSR_ESC;
    buf[1] = MSR_RESET;

    r = write (fd, buf, 2);

    usleep (100000);

    return 0;
}

int
msr_iso_read(fd)
{

    char buf[2];
    int r; 

    bzero (buf, sizeof(buf));

    buf[0] = MSR_ESC;
    buf[1] = MSR_ISO_READ;

    r = write (fd, buf, 2);

    usleep (100000);

    if (fd == -1)
        err(1, "Command write failed");
    else
        printf("wrote: %d\n", r);

    if (getstart (fd) == -1)
        err(1, "get start delimiter failed");

    gettrack (fd, 1);
    gettrack (fd, 2);
    gettrack (fd, 3);

    return 0;

}

int main(int argc, char * argv[])
{
	int fd = -1;
	int serial;

    /* Default device selection per platform */
    #ifdef __linux__ 
        char *device = "/dev/ttyS0";
    #else
        char *device = "/dev/cuaU0";
    #endif

	if (argv[1] != NULL)
		device = argv[1];
	else
		printf ("no device specified, defaulting to %s\n", device);

	/* bzero (buf, sizeof(buf)); */

    serial = serial_open (device, &fd);

    if (serial == -1) {
        err(1, "Serial open of %s failed", device);
        exit(1);
    }
    
    /* Prepare the reader with a reset */
    msr_reset (fd);

    /* Test the reader connection */
    comtest (fd);

    /* Get the device model */
    get_device_model (fd);

    /* Get the firmware version information */
    get_firmware_version (fd);

    /* Test the mag sensor */
    msr_sensor_test (fd);
    /* Flash the LEDs to make things more 31337 */
    printf("Preparing reader for reading...\n");
    /*do_leet_led_dance (fd);*/

    printf("Ready to read an ISO formatted card. Please slide a card.\n");

    /* Now we'll tell the reader we'd like to read the ISO formatted data */
    msr_iso_read (fd);

    /* We're finished */
	serial_close (fd);
	exit(0);
}
