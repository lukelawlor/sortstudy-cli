#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#include <ncurses.h>

#include "main.h"
#include "card.h"
#include "review.h"

#define	VERSION	"1.0.0"
#define	PRINT_ERROR(x)	fprintf(stderr, x)

static void print_help(void);

int main(int argc, char **argv)
{
	// Handle command line arguments
	if (argc == 1)
	{
		print_help();
		exit(EXIT_SUCCESS);
	}

	// Read card files if there are any in argv
	int filecount;

	if (argv[1][0] == '-')
		filecount = 0;
	else
	{
		char **filenames = argv + 1;
		filecount = 1;
		for (int i = 2; i < argc && argv[i][0] != '-'; i++)
			filecount++;
		if (read_deck(filenames, filecount) != 0)
			exit(EXIT_FAILURE);
	}
	
	// Handle options
	bool startup_shuffle = false;
	bool startup_noborder = false;
	bool startup_flip = false;
	for (int i = filecount + 1; i < argc; i++)
	{
		if (argv[i][1] != '\0' && argv[i][2] != '\0')
			goto option_error;

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
				case 'h':
					print_help();
					exit(EXIT_SUCCESS);
				default:
					goto option_error;
			}
		}
		continue;

		option_error:
		fprintf(stderr, "sortstudy-cli: unknown option \"%s\"\n", argv[i]);
		exit(EXIT_FAILURE);
	}

	// If no card files were read, exit
	if (filecount == 0)
	{
		PRINT_ERROR("no card files provided");
		fprintf(stderr, "sortstudy-cli: no card files provided\n");
		exit(EXIT_FAILURE);
	}

	// Init ncurses
	if (initscr() == NULL)
	{
		fprintf(stderr, "sortstudy-cli: failed to initialize ncurses\n");
		exit(EXIT_FAILURE);
	}

	// Don't draw pressed keys on the screen
	if (noecho() == ERR)
	{
		fprintf(stderr, "sortstudy-cli: noecho() failed\n");
		end_program(EXIT_FAILURE);
	}

	// Enable special keys for the standard screen
	if (keypad(stdscr, true) == ERR)
	{
		fprintf(stderr, "sortstudy-cli: keypad() failed\n");
		end_program(EXIT_FAILURE);
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

static void print_help(void)
{
	printf("Sort Study CLI, version %s\nusage: sortstudy-cli [card file(s)] [options]\noptions:\n\t-s\tshuffle cards at start\n\t-b\tdisable card borders at start\n\t-f\tflip cards at start\n\t-h\tdisplay this help text\nbasic review mode controls:\n\tJ\tflip card\n\tK\tmark card as wrong\n\tL\tmark card as right\n\tD\tdelete card\n\tB\ttoggle card borders\n\tQ\tquit\n\nSort Study CLI home page: <https://github.com/lukelawlor/sortstudy-cli>\n", VERSION);
}
