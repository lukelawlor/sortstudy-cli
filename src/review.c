#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

#include <ncurses.h>

#include "main.h"
#include "card.h"
#include "review_ui.h"
#include "review_act.h"
#include "review.h"

// Text
#define	REVIEW_FINISH_TEXT	"Review Complete!\n  Press N to start the next review\n  Press S to shuffle the cards\n  Press F to flip the cards"
#define	SMALL_WIN_TEXT		"This window is too small to run sort study"

// Minimum screen dimensions
#define	MIN_SCREEN_H		18
#define	MIN_SCREEN_W		35

// Macro to completely redraw the info window
#define	REDRAW_INFOWIN()	wclear(infowin);\
				draw_infowin();\
				wrefresh(infowin)

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
char lastaction[23];

// Sets numcards based on the amount of cards with the DO_REVIEW state
static void set_numcards(void);

// Toggles the drawing of borders of cards
static void toggle_borders(void);

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
	{
		endwin();
		fprintf(stderr, "sortstudycli: windows failed to initialize");
		exit(EXIT_FAILURE);
	}

	numcards = card_list_len;

	// Keeps track of whether or not the user has marked all cards right
	bool all_cards_right;

	// Review loop
	for (;;)
	{
		next_review:
		all_cards_right = true;
		strncpy(lastaction, "New review started", 19);
		cardpos = 0;
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
			REDRAW_INFOWIN();

			get_input:
			switch (tolower(wgetch(frontwin)))
			{
				case 'j':
					// Toggle back of card visibility
					if ((showback = showback ? false : true) == true)
						DRAW_BACKWIN();
					else
						wclear(backwin);
					wrefresh(backwin);
					goto get_input;
				case 'k':
					// Mark card as wrong
					review_list[i] = DO_REVIEW;
					all_cards_right = false;
					wrong_cards++;
					strncpy(lastaction, "Marked card wrong", 18);
					break;
				case 'l':
					// Mark card as right
					review_list[i] = DONT_REVIEW;
					right_cards++;
					strncpy(lastaction, "Marked card right", 18);
					break;
				case 'd':
					// Delete card
					if (card_list_len == 1)
					{
						strncpy(lastaction, "Can't delete last card", 23);
						REDRAW_INFOWIN();
						goto get_input;
					}

					review_list[i] = TO_DELETE;
					if (delete_marked_cards() == 0)
					{
						// Delete successful, decrement cardpos to not skip over the next card
						strncpy(lastaction, "Deleted card", 13);

						// Update global variables for drawing
						numcards--;

						cardpos--;

						// Decrement i so the next card isn't skipped over
						i--;
					}
					else
					{
						// Delete unsuccessful
						strncpy(lastaction, "Delete error", 13);
						review_list[i] = DO_REVIEW;
						REDRAW_INFOWIN();
						goto get_input;
					}
					break;
				case 'b':
					toggle_borders();
					goto get_input;
				case KEY_RESIZE:
					resize_window();
					goto get_input;
				case 'q':
					end_program(EXIT_SUCCESS);
				default:
					goto get_input;
			}
		}

		// If the user marked all cards as right this review, review every card again
		if (all_cards_right)
			for (int i = 0; i < card_list_len; i++)
				review_list[i] = DO_REVIEW;

		// Update global variables for drawing
		cardpos = 0;
		showback = false;
		review_finished = true;
		fronttext = REVIEW_FINISH_TEXT;
		set_numcards();
		is_full_review = numcards == card_list_len;

		// Show review finished screen

		REDRAW_INFOWIN();
		wclear(frontwin);
		DRAW_FRONTWIN();
		wrefresh(infowin);
		wclear(backwin);
		wrefresh(backwin);

		for (;;)
		{
			switch (tolower(wgetch(frontwin)))
			{
				case 'n':
					review_finished = false;
					goto next_review;
				case 'f':
					flip_cards();
					if (cards_flipped)
						strncpy(lastaction, "Flipped cards", 14);
					else
						strncpy(lastaction, "Unflipped cards", 16);
					REDRAW_INFOWIN();
					break;
				case 's':
					if (shuffle_cards() == 0)
						strncpy(lastaction, "Shuffled cards", 15);
					else
						strncpy(lastaction, "Shuffle calloc error", 21);
					REDRAW_INFOWIN();
					break;
				case 'q':
					end_program(EXIT_SUCCESS);
				case 'b':
					toggle_borders();
					break;
				case KEY_RESIZE:
					resize_window();
					break;
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
				end_program(EXIT_SUCCESS);
		}
		while (my < MIN_SCREEN_H || mx < MIN_SCREEN_W);

		// Clear the small window text and reinitialize the old windows
		wclear(stdscr);
		wrefresh(stdscr);

		if (init_windows() != 0)
		{
			endwin();
			fprintf(stderr, "sortstudycli: windows failed to initialize");
			exit(EXIT_FAILURE);
		}
	}
}

/*
 * set numcards (the total number of cards being reviewed) to the amount of DO_REVIEW values in review_list (declared in card.c)
 */
static void set_numcards(void)
{
	numcards = 0;
	for (int i = 0; i < card_list_len; i++)
		if (review_list[i] == DO_REVIEW)
			numcards++;
}

/*
 * toggles the drawing of borders around card windows
 */
static void toggle_borders(void)
{
	showborders = showborders ? false : true;
	wclear(frontwin);
	DRAW_FRONTWIN();
	wrefresh(frontwin);
	if (showback)
	{
		wclear(backwin);
		DRAW_BACKWIN();
		wrefresh(backwin);
	}
}
