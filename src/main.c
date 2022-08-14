/*
 * main.c
 *
 * This file contains the main function and functions that handle command line arguments
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <ncurses.h>

#include "main.h"
#include "card.h"
#include "review.h"

#define	VERSION	"1.0.1"

// Flags for startup actions in review mode
static bool startup_shuffle = false;
static bool startup_noborders = false;
static bool startup_flip = false;

// Print the text output when -h is passed
static void print_help(void);

// Handle verbose options (e.g. --shuffle)
static void handle_verbose_option(const char *str);

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
	{
		// The first argument is an option, so there were no files passed
		filecount = 0;
	}
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
	for (int i = filecount + 1; i < argc; i++)
	{
		if (argv[i][0] != '-')
			continue;
			
		if (argv[i][1] == '-')
		{
			handle_verbose_option(argv[i] + 2);
			continue;
		}

		if (argv[i][1] != '\0' && argv[i][2] != '\0')
		{
			// Non-verbose option found with more than one character after the hyphen
			fprintf(stderr, "sortstudycli: unknown option \"%s\"\n", argv[i]);
			exit(EXIT_FAILURE);
		}

		switch (argv[i][1])
		{
			case 's':
				startup_shuffle = true;
				break;
			case 'b':
				startup_noborders = true;
				break;
			case 'f':
				startup_flip = true;
				break;
			case 'h':
				print_help();
				exit(EXIT_SUCCESS);
			case 'v':
				printf("%s\n", VERSION);
				exit(EXIT_SUCCESS);
			default:
				fprintf(stderr, "sortstudycli: unknown option \"%s\"\n", argv[i]);
				exit(EXIT_FAILURE);
		}
	}

	// If no card files were read, exit
	if (filecount == 0)
	{
		fprintf(stderr, "sortstudycli: no card files provided\n");
		exit(EXIT_FAILURE);
	}

	// Init ncurses
	if (initscr() == NULL)
	{
		fprintf(stderr, "sortstudycli: failed to initialize ncurses\n");
		exit(EXIT_FAILURE);
	}

	// Don't draw pressed keys on the screen
	if (noecho() == ERR)
	{
		fprintf(stderr, "sortstudycli: ncurses noecho function failed\n");
		end_program(EXIT_FAILURE);
	}

	// Enable special keys for the standard screen
	if (keypad(stdscr, true) == ERR)
	{
		fprintf(stderr, "sortstudycli: ncurses keypad function failed\n");
		end_program(EXIT_FAILURE);
	}

	// Set random seed
	srand(time(NULL));

	start_review_mode(startup_shuffle, startup_noborders, startup_flip);
}

// Calls endwin and then exits the program
void end_program(const int exitcode)
{
	endwin();
	exit(exitcode);
}

// Print the text output when -h is passed
static void print_help(void)
{
	printf(
	"Sort Study CLI, version %s\n"
	"usage: sortstudycli cardfile [cardfile2 ...] [options]\n"
	"options:\n"
	"\t-s, --shuffle           shuffle cards at start\n"
	"\t-b, --no-borders        disable card borders at start\n"
	"\t-f, --flip              flip cards at start\n"
	"\t-h, --help              display this help text\n"
	"\t-v, --version           display version and exit\n"
	"basic review mode controls:\n"
	"\tJ\tflip card\n"
	"\tK\tmark card as wrong\n"
	"\tL\tmark card as right\n"
	"\tD\tdelete card\n"
	"\tQ\tquit\n\n"
	"Type \"man sortstudycli\" for more information\n"
	"Sort Study CLI home page: <https://github.com/lukelawlor/sortstudycli>\n"
	, VERSION);
}

/*
 * handles verbose options (e.g. --shuffle)
 *
 * str - string option excluding the "--"
 */
static void handle_verbose_option(const char *str)
{
	if (strcmp(str, "shuffle") == 0)
	{
		startup_shuffle = true;
		return;
	}
	else if (strcmp(str, "no-borders") == 0)
	{
		startup_noborders = true;
		return;
	}
	else if (strcmp(str, "flip") == 0)
	{
		startup_flip = true;
		return;
	}
	else if (strcmp(str, "help") == 0)
	{
		print_help();
		exit(EXIT_SUCCESS);
	}
	else if (strcmp(str, "version") == 0)
	{
		printf("%s\n", VERSION);
		exit(EXIT_SUCCESS);
	}

	fprintf(stderr, "sortstudycli: unknown option \"--%s\"", str);
	exit(EXIT_FAILURE);
}
