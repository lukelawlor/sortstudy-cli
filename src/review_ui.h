#define	DRAW_FRONTWIN()		draw_card_win(frontwin, fronttext)
#define	DRAW_BACKWIN()		draw_card_win(backwin, backtext)
extern WINDOW *infowin, *frontwin, *backwin;
extern char *fronttext, *backtext;
extern bool showback, showborders;
int init_windows(void);
void resize_window(void);
void draw_infowin(void);
void draw_card_win(WINDOW *, char *);
