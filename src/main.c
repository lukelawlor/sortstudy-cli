#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

#include <ncurses.h>

#include "card.h"

int main(void)
{
	// Init ncurses
	if (initscr() == NULL)
	{
		perror("failed to init ncurses");
		exit(errno);
	}
	noecho();

	read_deck("data/card.txt");

	for (;;)
	{
		bool showback = false;
		for (cardpos = 0; cardpos < card_list_len; cardpos++)
		{
			clear();
			mvprintw(0, 0, card_list[cardpos]->front);

			get_input:
			switch (tolower(getch()))
			{
				case 'j':
				{
					if ((showback = showback ? false : true) == true)
						mvprintw(2, 0, card_list[cardpos]->back);
					else
					{
						// Erase back of card from screen
						clear();
						mvprintw(0, 0, card_list[cardpos]->front);
					}
					goto get_input;
				}
				case 'k':	// fallthrough
				case 'l':
				{
					
					break;
				}
				case 'q':
				{
					goto close_program;
				}
				default:
					goto get_input;
			}
		}
	}

	close_program:
	endwin();
	exit(0);
}

