#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "card.h"

int main(void)
{
	read_card("data/card.txt");

	for (int i = 0; i < card_list_len; i++)
	{
		printf("Front: %s\nBack: %s\n\n", card_list[i]->front, card_list[i]->back);
	}

	exit(0);
}

