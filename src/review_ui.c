/*
 * review_ui.c
 *
 * This file contains functions for drawing individual pieces of the user interface of review mode.
 */

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <wchar.h>

// Include ncurses with wide character support
#define	_XOPEN_SOURCE_EXTENDED
#include <ncursesw/curses.h>

#include "util.h"
#include "review_ui.h"
#include "review.h"

// The width of the info window
#define INFO_WIN_H		3

// The minimum x position of centered info text
#define	MIN_INFO_CENTER_X	14

// The distance between the edges of the screen and the horizontal sides of the card windows
#define CARD_WIN_PADDING	2

// Get the y position of the front or back card based on the height of the screen
#define	GET_FRONT_WIN_Y(my)	(my - INFO_WIN_H) / 2 - 1 - card_win_h + INFO_WIN_H
#define	GET_BACK_WIN_Y(my)	(my - INFO_WIN_H) / 2 + INFO_WIN_H

// Get the x position of a card based on the width of the screen
#define	GET_CARD_WIN_X(mx)	mx / 2 - card_win_w / 2

WINDOW *infowin, *frontwin, *backwin;
wchar_t *fronttext, *backtext;

// Controls the visibility of the back of the card being viewed
bool showback = false;

// Controls the visibility of borders on all windows
bool showborders = true;

// Dimensions of card windows
static int card_win_h = 4;
static int card_win_w = 20;

/*
 * creates review mode windows
 * 
 * returns errno on error
 */
int init_windows(void)
{
	int mx, my;

	getmaxyx(stdscr, my, mx);
	infowin = newwin(INFO_WIN_H, mx, 0, 0);

	card_win_w = mx - CARD_WIN_PADDING * 2;
	card_win_h = (my - INFO_WIN_H) / 2 - 2;

	frontwin = newwin(card_win_h, card_win_w, GET_FRONT_WIN_Y(my), GET_CARD_WIN_X(mx));
	backwin = newwin(card_win_h, card_win_w, GET_BACK_WIN_Y(my), GET_CARD_WIN_X(mx));

	if (infowin == NULL || frontwin == NULL || backwin == NULL)
	{
		perror("newwin");
		return errno;
	}

	// Enable special key detection for frontwin, the window used for input
	if ((keypad(frontwin, true)) == ERR)
	{
		perror("keypad");
		return errno;
	}

	return 0;
}

void draw_infowin(void)
{
	// Print cardpos/numcards
	wmove(infowin, 0, 0);
	if (cardpos > MAX_INFO_CARDS)
		waddstr(infowin, "Card " STR(MAX_INFO_CARDS) "+/");
	else
		wprintw(infowin, "Card %d/", cardpos);
	if (numcards > MAX_INFO_CARDS)
		waddstr(infowin, STR(MAX_INFO_CARDS) "+\n");
	else
		wprintw(infowin, "%d\n", numcards);
	
	// Print right_cards and wrong_cards
	if (right_cards > MAX_INFO_RIGHTWRONG)
		waddstr(infowin, "Right: " STR(MAX_INFO_RIGHTWRONG) "+\n");
	else
		wprintw(infowin, "Right: %d\n", right_cards);
	if (wrong_cards > MAX_INFO_RIGHTWRONG)
		waddstr(infowin, "Wrong: " STR(MAX_INFO_RIGHTWRONG) "+\n");
	else
		wprintw(infowin, "Wrong: %d\n", wrong_cards);
	
	// Print the type of review and lastaction

	// Amount of characters to print for review type and lastaction text
	int review_chars = 6 + get_digits(numcards);
	int last_chars = strlen(lastaction);

	if (review_finished)
		review_chars += 6;
	if (is_full_review)
		review_chars += 12;
	else
		review_chars += 10;

	// Text-positioning variables

	// Width of the screen
	int screen_width = getmaxx(stdscr);

	// Width of half the entire text
	int half_text_width = MAX(review_chars, last_chars) / 2;

	// The x position to print centered text around
	int midx = (screen_width / 2);

	// Adjust midx when the centered text is close to the left side of the screen
	if (midx - half_text_width <= MIN_INFO_CENTER_X)
		midx = MIN_INFO_CENTER_X + half_text_width;

	// Printing review type
	wmove(infowin, 0, midx - (review_chars / 2));
	if (review_finished)
		waddstr(infowin, "Next: ");
	if (is_full_review)
		waddstr(infowin, "Full Review ");
	else
		waddstr(infowin, "Reviewing ");
	wprintw(infowin, "%d cards", numcards);

	// Printing lastaction
	wmove(infowin, 2, midx - (last_chars / 2));
	waddstr(infowin, lastaction);
}

/*
 * draws a card window with or without borders; this is used to draw the front and back of cards
 */
void draw_card_win(WINDOW *win, wchar_t *text)
{
	if (showborders)
	{
		wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);

		// Draw text within border
		bool full_text_drawn = false;
		int y, x, i;
		wchar_t c;

		i = 0;
		y = 1;
		x = 1;
		wmove(win, y, x);

		for (;;)
		{
			if ((c = text[i++]) == '\0')
			{
				full_text_drawn = true;
				break;
			}

			// Move 1 line down if a newline is read; don't print the newline
			if (c == '\n')
			{
				wmove(win, ++y, (x = 1));
				if (y > card_win_h - 2)
					break;
			}
			else
			{
				// Move 1 line down if the cursor reaches the edge of the screen
				if (x % (card_win_w - 1) == 0)
					wmove(win, ++y, (x = 1));
				if (y > card_win_h - 2)
					break;
				//waddch(win, c);
				wprintw(win, "%lc", c);
			}
			x++;
		}

		// Draw ">" on the border when the full text on the card is too large to be drawn
		if (!full_text_drawn)
			mvwaddch(win, card_win_h - 2, card_win_w - 1, '>');
	}
	else
	{
		//move(win, 0, 0);
		//waddwstr(win, text);
		mvwaddwstr(win, 0, 0, text);

		// Draw ">" for same reason as above
		if (wcslen(text) > (size_t) (card_win_w * card_win_h))
			mvwaddch(win, card_win_h - 1, card_win_w - 1, '>');
	}
}

/*
 * checks if the window's new dimensions are greater than or equal to MIN_H and MIN_W
 * 	if they aren't, wait for the user to resize the window to a proper size;
 * 	if they are, redraw the text on the screen and move the card windows to the center of the screen
 */
void resize_window(void)
{
	int mx, my;
	getmaxyx(stdscr, my, mx);

	prevent_small_screen(my, mx);

	wclear(infowin);
	wclear(frontwin);
	wclear(backwin);
	wrefresh(infowin);
	wrefresh(frontwin);
	wrefresh(backwin);

	card_win_w = mx - CARD_WIN_PADDING * 2;
	card_win_h = (my - INFO_WIN_H) / 2 - 2;

	mvwin(frontwin, GET_FRONT_WIN_Y(my), GET_CARD_WIN_X(mx));
	mvwin(backwin, GET_BACK_WIN_Y(my), GET_CARD_WIN_X(mx));
	wresize(frontwin, card_win_h, card_win_w);
	wresize(backwin, card_win_h, card_win_w);
	wresize(infowin, INFO_WIN_H, mx);

	draw_infowin();
	DRAW_FRONTWIN();
	wrefresh(infowin);
	wrefresh(frontwin);

	if (showback)
	{
		DRAW_BACKWIN();
		wrefresh(backwin);
	}
}
