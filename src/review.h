// Maximum number of cards printed in the info window
#define	MAX_INFO_CARDS		999

// Maximum number of cards marked right or wrong printed in the info window
#define	MAX_INFO_RIGHTWRONG	99999

// Counters for cards marked right and wrong
extern int right_cards, wrong_cards;

// The index of card_list being reviewed
extern int cardpos;

// The total number of cards being reviewed
extern int numcards;

// Text containing the last action performed by the user
extern char lastaction[23];

// True if the whole deck of cards is or will be reviewed
extern bool is_full_review;

// True if the review finished screen is being shown
extern bool review_finished;

// Starts review mode
void start_review_mode(bool startup_shuffle, bool startup_noborder, bool startup_flip);

// Checks if the screen's resolution is below the minimum allowed, and pauses the program's execution if so
void prevent_small_screen(int my, int mx);
