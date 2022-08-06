#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#include <ncurses.h>

#include "main.h"
#include "card.h"
#include "review.h"

int main(int argc, char **argv)
{
	// Handle command line arguments
	if (argc == 1)
	{
		endwin();
		fprintf(stderr, "usage: sortstudy-cli card_file [options]\n");
		exit(1);
	}

	// Read the card deck from argv
	if (read_deck(argv[1]) != 0)
		end_program(errno);
	
	// Handle options
	bool startup_shuffle, startup_noborder, startup_flip;
	if (argc > 2)
	{
		for (int i = 2; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				switch (argv[i][1])
				{
					case 's':
						startup_shuffle = true;
						break;
					case 'b':
						startup_noborder = true;
						break;
					case 'f':
						startup_flip = true;
						break;
						
				}
			}
		}
	}

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

	// Set random seed
	srand(time(NULL));

	start_review_mode(startup_shuffle, startup_noborder, startup_flip);
}

void end_program(int exitcode)
{
	endwin();
	exit(exitcode);
}
