#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

unsigned char rshift(unsigned char a, unsigned char b, int shift)
{
    return (((unsigned short)(a<<8 | b) >> (8 - shift)) & 0x00ff);
}

int main(int argc, char **argv)
{
	int fd = -1;
	ssize_t bytes_read = 0;
	unsigned char a, b, byte;

	int offset;

	if (argc < 3)
	{
		printf("Usage: %s [file] [offset]\n", argv[0]);
		exit(1);
	}

	sscanf(argv[2], "%d", &offset);

	if (offset < 0 || offset > 7)
	{
		printf("Error: Offset must be between 0 and 7 (inclusive).\n");
		exit(1);
	}

	printf("Offset: %d bits.\n", offset);

	fd = open(argv[1], O_RDONLY);

	read(fd, &a, 1);
	do
	{
		bytes_read = read(fd, &b, 1);
		byte = rshift(a, b, offset);
		write(2, &byte, 1);
		a = b;
	}
	while (bytes_read != 0);

	close(fd);

	return 0;
}
