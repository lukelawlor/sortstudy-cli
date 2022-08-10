// The estimated size for card_list when reading a deck
#define	CARD_ARRAY_ESTSIZE	100

// The maximum amount of characters read for each line in a card file
#define	MAX_LINE_CHARS		5000

// Card state values
#define	DONT_REVIEW	0
#define	DO_REVIEW	1
#define	TO_DELETE	2

// Card and card state types
typedef struct card{
	char *front;
	char *back;
} card_t;
typedef short cardstate_t;

// Array of card pointers
extern card_t **card_list;

// Length of card_list
extern int card_list_len;

// Array of card states
extern cardstate_t *review_list;

// Reads a deck of cards from one or more files
int read_deck(char **filenames, int filecount);

// Frees the a card array and all of its elements
void free_card_list(card_t **, int);

// Deletes cards in card_list that have the state TO_DELETE
int delete_marked_cards(void);
