#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <err.h>

#include "libmsr.h"

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

    buf[0] = MSR_ESC;
    buf[1] = MSR_COMMTEST;

    r = write (fd, buf, 2); 

    usleep (100000);

    if (fd == -1)
        err(1, "Commtest write failed");
    //else
    //    printf("Commtest wrote: %d\n", r);

    // read the result
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

int get_firmware_version (int fd)
{
    int r;
    char b[256];
    char buf[256];

    buf[0] = MSR_ESC;
    buf[1] = 0x76;

    r = write (fd, buf, 2);

    usleep (100000);

    if (fd == -1)
        err(1, "Firmware write failed");
    //else
    //    printf("Firmware version wrote: %d\n", r);

    // read the result "REV?X.XX"
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

    buf[0] = MSR_ESC;
    buf[1] = 0x74;

    r = write (fd, buf, 2);

    usleep (100000);

    if (fd == -1)
        err(1, "Device Model write failed");
    //else
    //    printf("Device Model wrote: %d\n", r);

    // read the result as the value of X in "MSR206-X"
    read (fd, &b, 3);
    b[4] = '\0';

    if (b[1] == '1' || '2' || '3' || '5')
    {
        printf("Device Model detected: MSR-206-%c\n", b[1]);
        return 0;
    }
    
    return 1;
}

int flash_led(int fd, char led)
{

    int r;
    char buf[2];

    buf[0] = MSR_ESC;
    buf[1] = led;

    r = write (fd, buf, 2);

    usleep (100000);

    if (fd == -1)
        err(1, "LED failure");

    // No response, look at the lights Dr. Love
    return 0;

}

int do_leet_led_dance(fd)
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

int setup_serial()
{
    // 
    return 0;
}


int isoread()
{
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

int main()
{
	int fd;
    struct termios  options;
	char    buf[256];
	int     i, r;

    // Default device selection per platform
    #ifdef linux 
        const char *device = "/dev/ttyS0";
    #elif BSD
        const char *device = "/dev/cuaU0";
    #endif

	bzero (buf, sizeof(buf));

    printf("Attempting to open %s\n", device);

	fd = open(device, O_RDWR | O_NOCTTY /*| O_NDELAY*/);

	if (fd == -1)
		err(1, "Open failed. Unable to open %s", device);

	printf("Device open successful...\n"); 
    printf("FD: %d\n", fd);

	tcgetattr(fd, &options);

	printf("got options...\n");

	/* Set baud rate */

	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;

	options.c_cflag |= CS8;

	options.c_lflag &= ~ICANON;
	options.c_lflag &= ~ICRNL;
	options.c_lflag &= ~ECHO;
	options.c_iflag |= IXOFF;

	options.c_oflag &= ~OPOST;
	tcsetattr(fd, TCSANOW, &options);
	printf("set options...\n");

    // Ensure the reader is reset
	buf[0] = MSR_ESC;
	buf[1] = MSR_RESET;

	r = write (fd, buf, 2);

	usleep (100000);

    // Test the reader connection
    comtest(fd);

    // Get the device model
    get_device_model(fd);

    // Get the firmware version information
    get_firmware_version(fd);

    // Flash the LEDs to make things more 31337
    printf("Preparing reader for reading...\n");
    do_leet_led_dance(fd);

    printf("Ready to read\n");

    // Now we'll tell the reader we'd like to read the ISO formatted data
	buf[0] = MSR_ESC;
	buf[1] = MSR_ISO_READ; 

	r = write (fd, buf, 2);

	usleep (100000);

	if (fd == -1)
		err(1, "write failed");
	else
		printf("wrote: %d\n", r);

	i = 0;

	if (getstart (fd) == -1)
		err(1, "get start delimiter failed");

	gettrack (fd, 1);
	gettrack (fd, 2);
	gettrack (fd, 3);

	close (fd);

	printf("buf: %s\n", buf);

	exit(0);
}
