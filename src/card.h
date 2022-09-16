// The estimated size for card_list when reading a deck
#define	CARD_ARRAY_ESTSIZE	100

// The maximum amount of characters read for each line in a card file
#define	MAX_LINE_CHARS		5000

// Card and card state types
typedef enum cardstate{
	CARDSTATE_DONT_REVIEW,
	CARDSTATE_DO_REVIEW,
	CARDSTATE_TO_DELETE
} cardstate_t;
typedef struct card{
	wchar_t *front;
	wchar_t *back;
	cardstate_t state;
} card_t;

// Array of card pointers
extern card_t **card_list;

// Length of card_list
extern int card_list_len;

// Reads a deck of cards from one or more files
int read_deck(char **filenames, int filecount);

// Frees a card from its pointer
void free_card(card_t *card);

// Frees the a card array and all of its elements
void free_card_list(card_t **, int);

// Deletes cards in card_list that have the state TO_DELETE
int delete_marked_cards(void);
