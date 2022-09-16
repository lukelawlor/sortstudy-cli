// Draw the front card window
#define	DRAW_FRONTWIN()		draw_card_win(frontwin, fronttext)

// Draw the back card window
#define	DRAW_BACKWIN()		draw_card_win(backwin, backtext)

// Info window, front of card window, and back of card window
extern WINDOW *infowin, *frontwin, *backwin;

// Text to show on the front and back card windows being drawn
extern wchar_t *fronttext, *backtext;

// True if the back of the card should be drawn
extern bool showback;

// True if the borders of card windows should be drawn
extern bool showborders;

// Initialize infowin, frontwin, and backwin; returns errno on error
int init_windows(void);

// Handles the resizing of the screen
void resize_window(void);

// Draws the info window
void draw_infowin(void);

// Draws a card window
void draw_card_win(WINDOW *win, wchar_t *text);
