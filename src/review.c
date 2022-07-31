#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <ncurses.h>

#include "main.h"
#include "card.h"
#include "review_ui.h"
#include "review.h"

#define	SMALL_WIN_TEXT		"This window is too small to run sort study"
#define	MIN_SCREEN_H		22
#define	MIN_SCREEN_W		22

// No. of cards marked right or wrong
int right_cards, wrong_cards;

// Index of current card being read
int cardpos;

// Number of cards in the current review
int numcards;

// Text containing last action made by the user to display in the info window
char lastaction[100];

void start_review_mode(void)
{
	{
		int my, mx;
		getmaxyx(stdscr, my, mx);
		prevent_small_screen(my, mx);
	}

	if (init_windows() != 0)
		end_program(errno);

	// Keeps track of whether or not the user has marked all cards right
	bool all_cards_right = true;

	// Review loop
	for (;;)
	{
		all_cards_right = true;
		for (int i = 0; i < card_list_len; i++)
		{
			// Don't display cards that haven't been marked for review
			if (review_list[i] == false)
				continue;
	
			// Update global variables used for drawing
			cardpos = i;
			numcards = card_list_len;
			fronttext = card_list[i]->front;
			backtext = card_list[i]->back;

			// Hide the back of the card when a new card is shown
			showback = false;

			// Draw the back of the card
			redraw_windows:
			wclear(backwin);
			if (showback)
				DRAW_BACKWIN();
			wrefresh(backwin);

			// Display the front of the card
			wclear(frontwin);
			DRAW_FRONTWIN();
			wrefresh(frontwin);

			// Display misc info
			wclear(infowin);
			draw_infowin();
			wrefresh(infowin);

			get_input:
			switch (tolower(wgetch(frontwin)))
			{
				case 'j':
				{
					// Toggle back of card visibility
					if ((showback = showback ? false : true) == true)
						DRAW_BACKWIN();
					else
						wclear(backwin);
					wrefresh(backwin);
					goto get_input;
				}
				case 'k':
				{
					// Mark card as wrong
					review_list[i] = true;
					all_cards_right = false;
					wrong_cards++;
					strncpy(lastaction, "Marked card wrong", 18);
					break;
				}
				case 'l':
				{
					// Mark card as right
					review_list[i] = false;
					right_cards++;
					strncpy(lastaction, "Marked card right", 18);
					break;
				}
				case 'b':
				{
					showborders = showborders ? false : true;
					goto redraw_windows;
				}
				case KEY_RESIZE:
				{
					resize_window();
					goto get_input;
				}
				case 'q':
				{
					end_program(0);
				}
				default:
					goto get_input;
			}
		}

		// If the user marked all cards as right this review, review every card again
		if (all_cards_right)
			for (int i = 0; i < card_list_len; i++)
				review_list[i] = true;
	}
}

/*
 * checks if the screen dimensions are below MIN_W or MIN_H;
 * 	if they are, wait until the user resizes the screen to proper dimensions
 *
 * args:
 * 	my - screen height
 * 	mx - screen width
 */
void prevent_small_screen(int my, int mx)
{
	if (my < MIN_SCREEN_H || mx < MIN_SCREEN_W)
	{
		// Delete every window other than stdscr so stdscr can be used
		delwin(infowin);
		delwin(frontwin);
		delwin(backwin);

		// Enable special keys on stdscr so KEY_RESIZE can be listened for
		if (keypad(stdscr, true) == ERR)
		{
			perror("keypad");
			end_program(errno);
		}

		// Show the small window text and wait for the user to resize
		// the window so that the dimensions are >=  MIN_H and MIN_W
		int c;
		do
		{
			wclear(stdscr);
			mvwaddstr(stdscr, 0, 0, SMALL_WIN_TEXT);
			wrefresh(stdscr);
			if ((c = wgetch(stdscr)) == KEY_RESIZE)
				getmaxyx(stdscr, my, mx);
			else if (c == 'q')
				end_program(0);
		}
		while (my < MIN_SCREEN_H || mx < MIN_SCREEN_W);

		// Clear the small window text and reinitialize the old windows
		wclear(stdscr);
		wrefresh(stdscr);

		if (init_windows() != 0)
			end_program(errno);
	}
}
