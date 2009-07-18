#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_CARD_LEN 128
#define STRIDE_LEN 8
#define BYTES_PER_LINE 8

unsigned char rshift(unsigned char a, unsigned char b, int shift);
unsigned char lshift(unsigned char a, unsigned char b, int shift);
void rbump(unsigned char *buffer);
void lbump(unsigned char *buffer);
int read_source_card (const char *infile, unsigned char *buffer);
void print_bits(unsigned char c);
void display_card(const unsigned char *buffer);

int main(int argc, char **argv)
{
	unsigned char source_card[MAX_CARD_LEN];
	unsigned char offset_card[MAX_CARD_LEN];
	unsigned char command;

	memset(source_card, 0, MAX_CARD_LEN);

	if (argc < 2)
	{
		printf("Usage: %s [file]\n", argv[0]);
		exit(1);
	}

	printf("Parsing file.\n");

	read_source_card(argv[1], source_card);
	memcpy(offset_card, source_card, MAX_CARD_LEN);
	display_card(offset_card);

	printf("File parsed.\n");

	printf("> ");
	while (1)
	{
		command = getchar();

		switch (command)
		{
		case '\n':
			/* do nothing */
			printf("> ");
			continue;
		case 'q':
			/* quit */
			return 0;
		case 'j':
			/* next card */
			printf("Unimplemented.\n");
			break;
		case 'k':
			/* prev card */
			printf("Unimplemented.\n");
			break;
		case 'h':
			/* shift left */
			lbump(offset_card);
			display_card(offset_card);
			break;
		case 'l':
			/* shift right */
			rbump(offset_card);
			display_card(offset_card);
			break;
		case '?':
			/* print some help */
			printf("q - exit program\n");
			printf("? - this help screen\n");
			printf("j - next card\n");
			printf("k - previous card\n");
			printf("h - bit shift left\n");
			printf("l - bit shift right\n");
			break;
		default:
			/* warning */
			printf("Unrecognized command.\n");
			break;
		}
	}

	return 0;
}

unsigned char rshift(unsigned char a, unsigned char b, int shift)
{
    return (((unsigned short)(a<<8 | b) >> shift) & 0x00ff);
}

unsigned char lshift(unsigned char a, unsigned char b, int shift)
{
    return (((unsigned short)(a<<8 | b) >> (8 - shift)) & 0x00ff);
}

void rbump(unsigned char *buffer)
{
	unsigned char a, b, byte;
	int i;

	a = 0;
	for (i=0; i<MAX_CARD_LEN; i++)
	{
		b = *(buffer + i);
		byte = rshift(a, b, 1);
		*(buffer + i) = byte;
		a = b;
	}
}

void lbump(unsigned char *buffer)
{
	unsigned char a, b, byte;
	int i;

	a = *buffer;
	for (i=0; i<MAX_CARD_LEN; i++)
	{
		b = *(buffer + i + 1);
		byte = lshift(a, b, 1);
		*(buffer + i) = byte;
		a = b;
	}
}

int read_source_card (const char *infile, unsigned char *buffer)
{
	int fd = -1;
	ssize_t bytes_read = 0;
	unsigned char *cur_posn;
	
	fd = open(infile, O_RDONLY);

	cur_posn = buffer;
	do
	{
		bytes_read = read(fd, buffer, STRIDE_LEN);
		cur_posn += STRIDE_LEN;
	}
	while (bytes_read > 0);
	cur_posn = 0;

	close(fd);

	return bytes_read;
}

void print_bits(unsigned char c)
{
    int i;
    for (i = 0; i<8; i++)
    {  
        printf("%c", (c&0x80)?'1':'0');
        c <<= 1;
    }
	printf(" ");
}

void display_card(const unsigned char *buffer)
{
	int i;

	for (i=0; i<MAX_CARD_LEN; i++)
	{
		print_bits(*(buffer +i));

		if ((i + 1) % BYTES_PER_LINE == 0)
		{
			printf("\n");
		}
		else if ((i + 1) % 4 == 0)
		{
			printf(" ");
		}
	}
}

