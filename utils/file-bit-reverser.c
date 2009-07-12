#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

unsigned char reverse_byte(unsigned char byte)
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

int main(int argc, char **argv)
{
	int fd = -1;
	off_t filepos;
	unsigned char byte;

	if (argc < 2)
	{
		printf("Usage: %s [file]\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_RDONLY);

	filepos = lseek(fd, 0, SEEK_END);

	while (filepos > 0)
	{
		filepos = lseek(fd, filepos - 1, SEEK_SET);
		read(fd, &byte, 1);
		byte = reverse_byte(byte);
		write(2, &byte, 1);
	}

	close(fd);

	return 0;
}
