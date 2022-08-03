#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <ncurses.h>

#include "main.h"
#include "card.h"
#include "review.h"

int main(int argc, char **argv)
{
	// Handle command line argument errors
	if (argc != 2)
	{
		endwin();
		fprintf(stderr, "usage: sortstudy CARD_FILE\n");
		exit(1);
	}

	// Read the card deck from argv
	if (read_deck(argv[1]) != 0)
		end_program(errno);

	// Init ncurses
	if (initscr() == NULL)
	{
		perror("failed to initialize ncurses");
		exit(errno);
	}

	// Don't draw pressed keys on the screen
	if (noecho() == ERR)
	{
		perror("noecho");
		end_program(errno);
	}

	// Enable special keys for the standard screen
	if (keypad(stdscr, true) == ERR)
	{
		perror("keypad");
		end_program(errno);
	}

	start_review_mode();
}

void end_program(int exitcode)
{
	endwin();
	exit(exitcode);
}
