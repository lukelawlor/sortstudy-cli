#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <ncurses.h>

#include "main.h"
#include "card.h"
#include "review_ui.h"
#include "review_act.h"
#include "review.h"

#define	REVIEW_FINISH_TEXT	"Review Complete!\n  Press L to start the next review\n  Press S to shuffle the cards\n  Press F to flip the cards"
#define	SMALL_WIN_TEXT		"This window is too small to run sort study"
#define	MIN_SCREEN_H		22
#define	MIN_SCREEN_W		24

// No. of cards marked right or wrong
int right_cards, wrong_cards;

// Index of current card being read
int cardpos;

// Number of cards in the current review
int numcards;

// True if the review covers all cards
bool is_full_review = true;

// True if the review finished screen is being shown
bool review_finished = false;

// Text containing last action made by the user to display in the info window
char lastaction[21];

static void set_numcards(void);

void start_review_mode(bool startup_shuffle, bool startup_noborder, bool startup_flip)
{
	// Perform startup actions
	if (startup_shuffle)
		shuffle_cards();
	
	if (startup_noborder)
		showborders = false;
	
	if (startup_flip)
		flip_cards();
	
	// Check if the screen is too small
	{
		int my, mx;
		getmaxyx(stdscr, my, mx);
		prevent_small_screen(my, mx);
	}

	if (init_windows() != 0)
		end_program(errno);

	numcards = card_list_len;

	// Keeps track of whether or not the user has marked all cards right
	bool all_cards_right;

	// Review loop
	for (;;)
	{
		next_review:
		cardpos = 0;
		all_cards_right = true;
		strncpy(lastaction, "New review started", 19);
		for (int i = 0; i < card_list_len; i++)
		{
			// Don't display cards that haven't been marked for review
			if (review_list[i] != DO_REVIEW)
				continue;
	
			// Update global variables used for drawing
			cardpos++;
			fronttext = card_list[i]->front;
			backtext = card_list[i]->back;

			// Hide the back of the card when a new card is shown
			showback = false;

			// Draw the back of the card
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
					review_list[i] = DONT_REVIEW;
					all_cards_right = false;
					wrong_cards++;
					strncpy(lastaction, "Marked card wrong", 18);
					break;
				}
				case 'l':
				{
					// Mark card as right
					review_list[i] = DO_REVIEW;
					right_cards++;
					strncpy(lastaction, "Marked card right", 18);
					break;
				}
				case 'b':	// fall through
				{
					showborders = showborders ? false : true;
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
		{
			for (int i = 0; i < card_list_len; i++)
				review_list[i] = DO_REVIEW;
		}

		set_numcards();
		is_full_review = numcards == card_list_len;

		// Show review finished screen
		review_finished = true;
		wclear(infowin);
		wclear(frontwin);
		wclear(backwin);
		fronttext = REVIEW_FINISH_TEXT;
		draw_infowin();
		DRAW_FRONTWIN();
		wrefresh(infowin);
		wrefresh(backwin);

		for (;;)
		{
			switch (tolower(wgetch(frontwin)))
			{
				case 'l':
				{
					review_finished = false;
					goto next_review;
				}
				case 'f':
				{
					flip_cards();
					if (cards_flipped)
						strncpy(lastaction, "Flipped cards", 14);
					else
						strncpy(lastaction, "Unflipped cards", 16);
					wclear(infowin);
					draw_infowin();
					wrefresh(infowin);
					break;
				}
				case 's':
				{
					if (shuffle_cards() == 0)
						strncpy(lastaction, "Shuffled cards", 15);
					else
						strncpy(lastaction, "Shuffle calloc error", 21);
					wclear(infowin);
					draw_infowin();
					wrefresh(infowin);
					break;
				}
				case 'q':
				{
					end_program(0);
				}
				case 'b':	// fall through
				{
					showborders = showborders ? false : true;
				}
				case KEY_RESIZE:
				{
					resize_window();
					break;
				}
			}
		}
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

/*
 * set numcards (the total number of cards being reviewed) to the amount of true values in review_list (declared in card.c)
 */
static void set_numcards(void)
{
	numcards = 0;
	for (int i = 0; i < card_list_len; i++)
		numcards += review_list[i];
}
