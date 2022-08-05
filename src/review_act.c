#include <stdbool.h>

#include "card.h"
#include "review_act.h"

bool cards_flipped = false;

void flip_cards(void)
{
	char *temp;
	for (int i = 0; i < card_list_len; i++)
	{
		temp = card_list[i]->front;
		card_list[i]->front = card_list[i]->back;
		card_list[i]->back = temp;
	}
	cards_flipped = cards_flipped ? false : true;
}
