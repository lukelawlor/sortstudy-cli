#include <stdlib.h>
#include <errno.h>

#include <ncurses.h>

#include "main.h"
#include "card.h"
#include "review.h"

int main(void)
{
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

	read_deck("data/card.txt");
	start_review_mode();
}

void end_program(int exitcode)
{
	endwin();
	exit(exitcode);
}
