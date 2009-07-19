#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ncurses.h>
#include <errno.h>

#define MAX_FILENAME_LEN 32
#define MAX_CARD_LEN 128
#define STRIDE_LEN 8
#define BYTES_PER_LINE 8

/* Card List item structure */

typedef struct card_list {
	char filename[MAX_FILENAME_LEN];
	unsigned char raw_card[MAX_CARD_LEN];
	unsigned char mod_card[MAX_CARD_LEN];
	int offset;

	struct card_list *next;
	struct card_list *prev;
} CARD_LIST;

/* Card List manipulation functions */

CARD_LIST *card_list_create();
void card_list_destroy(CARD_LIST *item);
CARD_LIST *card_list_insert(CARD_LIST *item);
void card_list_remove(CARD_LIST *item);
CARD_LIST *add_card(const char *filename, CARD_LIST *cur_item);
CARD_LIST *remove_card(CARD_LIST *item);

/*  Card manipulation functions */

void display_card(CARD_LIST *item);
void shift_card(CARD_LIST *item, int shift);
int read_file (const char *filename, unsigned char *buffer);
void dump_buffer(const unsigned char *buffer);
void print_bits(unsigned char c);

/* Miscellaneous functions */

void display_header();
void display_footer();

int main(int argc, char **argv)
{
	CARD_LIST *cards;
	CARD_LIST *cur_card;
	unsigned char loop;
	int command;
	int i;

	if (argc <2)
	{
		printf("Usage: %s [file1] ... [fileN]\n", argv[0]);
		exit(1);
	}

	initscr();
	keypad(stdscr, TRUE);
	noecho();

	display_header();

	cards = NULL;
	for (i=1; i<argc; i++)
	{
		cards = add_card(argv[i], cards);
	}
	cur_card = cards->next;
	sleep(1);

	loop = 1;
	while (loop)
	{
		clear();
		display_header();
		display_card(cur_card);
		display_footer();
		printw("\n");
		refresh();

		command = getch();

		switch (command)
		{
		case KEY_LEFT:
			/* shift left */
			shift_card(cur_card, -1);
			break;
		case KEY_RIGHT:
			/* shift right */
			shift_card(cur_card, 1);
			break;
		case KEY_DOWN:
			/* next card */
			cur_card = cur_card->next;
			break;
		case KEY_UP:
			/* prev card */
			cur_card = cur_card->prev;
			break;
		case 'a':
			/* add card */
			cur_card = add_card(NULL, cur_card);
			break;
		case 'r':
			/* remove card */
			cur_card = remove_card(cur_card);
			break;
		case '?':
			/* print some help */
			printw("<DOWN> - next card\n");
			printw("<UP> - previous card\n");
			printw("<LEFT> - bit shift left\n");
			printw("<RIGHT> - bit shift right\n");
			printw("a - add card\n");
			printw("r - remove card\n");
			printw("? - this help screen\n");
			printw("q - exit program\n");
			printw("\n");
			printw("Press any key to continue ...\n");
			refresh();
			getch();
			break;
		case 'q':
			/* quit */
			loop = 0;
			break;
		case '\n':
			/* do nothing */
			continue;
		default:
			/* warning */
			printw("Unrecognized command: %c.\n", command);
			refresh();
			sleep(1);
			break;
		}

		refresh();
	}

	card_list_destroy(cards);

	endwin();

	return 0;
}

/*
 * Allocate a single CARD_LIST item.
 * Return the zeroed new list item.
 */
CARD_LIST *card_list_create()
{
	return (CARD_LIST *)calloc(1, sizeof(CARD_LIST));
}

/*
 * Deallocate an entire list of CARD_LIST items.
 */
void card_list_destroy(CARD_LIST *item)
{
	CARD_LIST *next;

	next = item;
	do
	{
		item = next;
		next = item->next;
		card_list_remove(item);
	}
	while (next != item);
}

/*
 * Create a single CARD_LIST item, and
 * insert it after the given list item.
 * Return the newly created item.
 */
CARD_LIST *card_list_insert(CARD_LIST *item)
{
	CARD_LIST *new_item = card_list_create();

	item->next->prev = new_item;
	new_item->prev = item;
	new_item->next = item->next;
	item->next = new_item;

	return new_item;
}

/*
 * Unlink the given CARD_LIST item, and
 * deallocate it.
 */
void card_list_remove(CARD_LIST *item)
{
	item->next->prev = item->prev;
	item->prev->next = item->next;
	memset(item, 0, sizeof(CARD_LIST));
	free(item);
}

/*
 * Create a new CARD_LIST item,
 * load it with data from the given filename,
 * and insert it after the given list item.
 * Return the newly created item.
 *
 * If the given filename is NULL,
 * then prompt the user for a filename.
 *
 * If the given list item is NULL,
 * then start a new empty list.
 *
 * If the specified file does not exist,
 * then just return the given list item.
 */
CARD_LIST *add_card(const char *filename, CARD_LIST *cur_item)
{
	char *fn;
	char buffer[MAX_FILENAME_LEN];
	int status;
	CARD_LIST *new_item;

	if (filename == NULL)
	{
		memset(buffer, 0, MAX_FILENAME_LEN);
		printw("Filename: ");
		refresh();
		echo();
		getstr(buffer);
		noecho();

		fn = buffer;
	}
	else
	{
		fn = (char *)filename;
	}

	if (cur_item == NULL)
	{
		new_item = card_list_create();
		new_item->next = new_item;
		new_item->prev = new_item;
	}
	else
	{
		new_item = card_list_insert(cur_item);
	}

	strcpy(new_item->filename, fn);
	status = read_file(fn, new_item->raw_card);

	if (status == -1)
	{
		card_list_remove(new_item);
		new_item = cur_item;

		printw("Could not load \"%s\".\n", fn);
		refresh();
		sleep(1);
	}
	else
	{
		memcpy(new_item->mod_card, new_item->raw_card, MAX_CARD_LEN);
		new_item->offset = 0;

		printw("Loaded \"%s\".\n", fn);
		refresh();
	}

	return new_item;
}

/*
 * Remove the given item from its list.
 * Return the next list item.
 */
CARD_LIST *remove_card(CARD_LIST *item)
{
	CARD_LIST *next;

	next = item->next;

	if (item == next)
	{
		printw("You cannot remove the only remaining card!\n");
	}
	else
	{
		printw ("Removed \"%s\".\n\n", item->filename);
		card_list_remove(item);
	}

	refresh();
	sleep(1);

	return next;
}

/*
 * Print a card's metadata and binary data.
 */
void display_card(CARD_LIST *item)
{
	printw("File: %s  (Offset: %+d bits)\n\n", item->filename, item->offset);
	dump_buffer(item->mod_card);
	refresh();
}

/*
 * Shift a card an arbitrary number of bits in either direction.
 *
 * Generates a shifted copy based on the original data every time.
 */
void shift_card(CARD_LIST *item, int shift)
{
	unsigned char a, b;
	unsigned char *src, *dst;
	int bit_offset, byte_offset;
	unsigned char rel_shift;
	int i;

	src = item->raw_card;
	dst = item->mod_card;

	memset(dst, 0, MAX_CARD_LEN);

	item->offset += shift;
	byte_offset = abs(item->offset) / 8;
	bit_offset = abs(item->offset) % 8;

	if (item->offset >= 0)
	{
		dst += byte_offset;
		rel_shift = bit_offset;
		a = 0;
	}
	else
	{
		byte_offset += 1;
		src += byte_offset;
		rel_shift = 8 - bit_offset;
		a = *(src - 1);
	}

	for (i=0; i<MAX_CARD_LEN - byte_offset; i++)
	{
		b = *(src + i);
		*(dst + i) = (((a<<8 | b) >> rel_shift ) & 0xff);
		a = b;
	}
}

/*
 * Read the contents of a file into a buffer.
 */
int read_file (const char *filename, unsigned char *buffer)
{
	int fd;
	ssize_t bytes_read;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		printw("open: %s.\n", strerror(errno));
		refresh();
		sleep(1);
		return -1;
	}

	do
	{
		bytes_read = read(fd, buffer, STRIDE_LEN);
		buffer += STRIDE_LEN;
	}
	while (bytes_read > 0);
	buffer = NULL;

	close(fd);

	return bytes_read;
}

/*
 * Print a buffer as reasonably formatted raw bits.
 */
void dump_buffer(const unsigned char *buffer)
{
	int i;

	for (i=0; i<MAX_CARD_LEN; i++)
	{
		print_bits(*(buffer +i));
		printw(" ");

		if ((i + 1) % BYTES_PER_LINE == 0)
		{
			printw("\n");
		}
		else if ((i + 1) % 4 == 0)
		{
			printw(" ");
		}
	}
}

/*
 * Print a byte as a string of eight bits.
 */
void print_bits(unsigned char c)
{
    int i;
    for (i = 0; i<8; i++)
    {  
        printw("%c", (c&0x80)?'1':'0');
        c <<= 1;
    }
}

/*
 * Print the header.
 */
void display_header()
{
	int y, x;
	char title[] = "File Field Visualizer 0.1";

	y = 0;
	x = (72 - strlen(title)) / 2;

	mvprintw(y, x, "%s\n", title);
	printw("\n");
	refresh();
}

/*
 * Print the footer.
 */
void display_footer()
{
	char commands[] = "Commands: <L>, <R>, <U>, <D>, a, r, ?, q.";
	printw("\n");
	printw("%s\n", commands);
	refresh();
}
