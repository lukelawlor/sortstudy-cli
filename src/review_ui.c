#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <ncurses.h>

#include "util.h"
#include "review_ui.h"
#include "review.h"

#define INFO_WIN_H		7
#define INFO_WIN_W		30
#define CARD_WIN_PADDING	3

// Get the y position of the front or back card based on the height of the screen
#define	GET_FRONT_WIN_Y(my)	(my - INFO_WIN_H) / 2 - 1 - card_win_h + INFO_WIN_H
#define	GET_BACK_WIN_Y(my)	(my - INFO_WIN_H) / 2 + INFO_WIN_H

// Get the x position of a card based on the width of the screen
#define	GET_CARD_WIN_X(mx)	mx / 2 - card_win_w / 2

WINDOW *infowin, *frontwin, *backwin;
char *fronttext, *backtext;

// Controls the visibility of the back of the card being viewed
bool showback = false;

// Controls the visibility of borders on all windows
bool showborders = false;

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
	infowin = newwin(INFO_WIN_H, INFO_WIN_W, 0, 0);

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
	if (showborders)
	{
		mvwprintw(infowin, 1, 1, "Card %d/%d\n Right: %d\n Wrong: %d\n\n %s", cardpos + 1, numcards, right_cards, wrong_cards, lastaction);
		wborder(infowin, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	else
		mvwprintw(infowin, 0, 0, "Card %d/%d\nRight: %d\nWrong: %d\n\n%s", cardpos + 1, numcards, right_cards, wrong_cards, lastaction);
}

/*
 * draws a card window with or without borders; this is used to draw the front and back of cards
 */
void draw_card_win(WINDOW *win, char *text)
{
	if (showborders)
	{
		wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);

		// Draw text within border
		bool full_text_drawn = false;
		int c, y, x, i;

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
				waddch(win, c);
			}
			x++;
		}

		// Draw ">" on the border when the full text on the card is too large to be drawn
		if (!full_text_drawn)
			mvwaddch(win, card_win_h - 2, card_win_w - 1, '>');
	}
	else
	{
		mvwaddstr(win, 0, 0, text);

		// Draw ">" for same reason as above
		if (strlen(text) > (card_win_w * card_win_h))
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
