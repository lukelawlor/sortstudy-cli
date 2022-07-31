#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

#include <ncurses.h>

#include "main.h"
#include "card.h"
#include "review.h"

#define	SMALL_WIN_TEXT	"This window is too small to run sort study"
#define INFO_WIN_H	6
#define INFO_WIN_W	20
#define	MIN_H		22
#define	MIN_W		22

// Get the y position of the front or back card based on the height of the screen
#define	GET_FRONT_WIN_Y(my)	(my - INFO_WIN_H) / 2 - 1 - card_win_h + INFO_WIN_H
#define	GET_BACK_WIN_Y(my)	(my - INFO_WIN_H) / 2 + INFO_WIN_H

// Get the x position of a card based on the width of the screen
#define	GET_CARD_WIN_X(mx)	mx / 2 - card_win_w / 2

#define	DRAW_FRONTWIN()		draw_card_win(frontwin, fronttext)
#define	DRAW_BACKWIN()		draw_card_win(backwin, backtext)

static WINDOW *infowin, *frontwin, *backwin;
static char *fronttext, *backtext;

// Controls the visibility of the back of the card being viewed
static bool showback = false;

// Controls the visibility of borders on all windows
static bool showborders = true;

// No. of cards marked right or wrong
static int right_cards, wrong_cards;

// Index of current card being read
static int cardpos;

// Dimensions of card windows
static int card_win_h = 4;
static int card_win_w = 20;

static int init_windows(void);
static void resize_window(void);
static void draw_infowin(void);
static void draw_card_win(WINDOW *win, char *text);

void start_review_mode(void)
{
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
			fronttext = card_list[i]->front;
			backtext = card_list[i]->back;

			// Hide the back of the card
			showback = false;
			wclear(backwin);
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
					break;
				}
				case 'l':
				{
					// Mark card as right
					review_list[i] = false;
					right_cards++;
					break;
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
 * creates review mode windows
 * 
 * returns errno on error
 */
static int init_windows(void)
{
	int mx, my;

	getmaxyx(stdscr, my, mx);
	infowin = newwin(INFO_WIN_H, INFO_WIN_W, 0, 0);

	card_win_w = mx - 10;
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

/*
 * checks if the window's new dimensions are greater than or equal to MIN_H and MIN_W
 * 	if they aren't, wait for the user to resize the window to a proper size;
 * 	if they are, redraw the text on the screen and move the card windows to the center of the screen
 */
static void resize_window(void)
{
	int mx, my;
	getmaxyx(stdscr, my, mx);

	// Check if the window's dimensions are below its minimum width and height
	if (my < MIN_H || mx < MIN_W)
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
		do
		{
			wclear(stdscr);
			mvwaddstr(stdscr, 0, 0, SMALL_WIN_TEXT);
			wrefresh(stdscr);
			if (wgetch(stdscr) == KEY_RESIZE)
				getmaxyx(stdscr, my, mx);
		}
		while (my < MIN_H || mx < MIN_W);

		// Clear the small window text and reinitialize the old windows
		wclear(stdscr);
		wrefresh(stdscr);

		if (init_windows() != 0)
			end_program(errno);
	}

	wclear(infowin);
	wclear(frontwin);
	wclear(backwin);
	wrefresh(infowin);
	wrefresh(frontwin);
	wrefresh(backwin);

	card_win_w = mx - 10;
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

static void draw_infowin(void)
{
	if (showborders)
	{
		mvwprintw(infowin, 1, 1, "card %d/%d\n right: %d\n wrong: %d", cardpos + 1, card_list_len, right_cards, wrong_cards);
		wborder(infowin, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	else
		mvwprintw(infowin, 0, 0, "card %d/%d\nright: %d\nwrong: %d", cardpos + 1, card_list_len, right_cards, wrong_cards);
}

static void draw_card_win(WINDOW *win, char *text)
{
	if (showborders)
	{
		wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);

		// Draw text within border
		bool full_text_drawn = false;
		int c;
		wmove(win, 1, 1);
		for (int i = 0; i < ((card_win_w - 2) * (card_win_h - 2)); i++)
		{
			if ((c = text[i]) == '\0')
			{
				full_text_drawn = true;
				break;
			}

			if (i % (card_win_w - 2) == 0)
				wmove(win, 1 + (i / (card_win_w - 2)), 1);
			waddch(win, c);
		}

		// Draw ">" on the border when the full text on the card is too large to be drawn
		if (!full_text_drawn)
			waddch(win, '>');
	}
	else
		mvwaddstr(win, 0, 0, text);
}
