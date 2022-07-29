#define	CARD_ARRAY_ESTSIZE	100
#define	MAX_LINE_CHARS		1000
typedef struct card{
	char *front;
	char *back;
} card_t;
extern card_t **card_list;
extern int card_list_len, cardpos;
int read_deck(char *);
void free_card_list(card_t **, int);
