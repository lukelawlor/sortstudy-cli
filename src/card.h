#define	CARD_ARRAY_ESTSIZE	100
#define	MAX_LINE_CHARS		1000
// Card states
#define	DONT_REVIEW	0
#define	DO_REVIEW	1
#define	TO_DELETE	2
typedef struct card{
	char *front;
	char *back;
} card_t;
typedef short cardstate_t;
extern card_t **card_list;
extern int card_list_len;
extern cardstate_t *review_list;
int read_deck(char **, int);
void free_card_list(card_t **, int);
int delete_marked_cards(void);
