#define	CARD_ARRAY_ESTSIZE	100
#define	MAX_LINE_CHARS		1000
typedef struct card{
	char *front;
	char *back;
} card_t;
extern card_t **card_list;
extern int card_list_len;
int read_card(char *);
void free_cards_in_list(card_t **, int);
