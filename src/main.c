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
		fprintf(stderr, "usage: sortstudy-cli [card file(s)] [options]\n");
		exit(1);
	}

	// Read the card deck from files in argv
	char **filenames = argv + 1;
	int filecount = 1;

	for (int i = 2; i < argc && argv[i][0] != '-'; i++)
		filecount++;

	if (read_deck(filenames, filecount) != 0)
		end_program(errno);
	
	// Handle options
	bool startup_shuffle = false;
	bool startup_noborder = false;
	bool startup_flip = false;
	for (int i = filecount + 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			// Check if the option is longer than 2 characters
			if (argv[i][1] != '\0' && argv[i][2] != '\0')
				goto option_error;

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
				default:
					goto option_error;
			}
		}
		continue;

		option_error:
		fprintf(stderr, "sortstudy-cli: unknown option \"%s\"\n", argv[i]);
		end_program(errno = EIO);
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
