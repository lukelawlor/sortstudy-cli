/*
 * review_act.c
 *
 * This file contains functions for manipulating cards in review mode through special user actions.
 *
 * These include flipping and shuffling cards.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>

#include "card.h"
#include "review_act.h"

bool cards_flipped = false;

/*
 * swaps the back text of cards with the front text
 */
void flip_cards(void)
{
	wchar_t *temp;
	for (int i = 0; i < card_list_len; i++)
	{
		temp = card_list[i]->front;
		card_list[i]->front = card_list[i]->back;
		card_list[i]->back = temp;
	}
	cards_flipped = cards_flipped ? false : true;
}

/*
 * shuffles the order of cards while preserving what cards need to be reviewed
 *
 * returns errno on error, but doesn't print errors like read_deck
 */
int shuffle_cards(void)
{
	// Create an array with shuffled indexes pointed to by new_indexes
	int *new_indexes, *old_indexes, index;

	if ((new_indexes = calloc(card_list_len, sizeof(int))) == NULL)
		return errno;
	if ((old_indexes = calloc(card_list_len, sizeof(int))) == NULL)
	{
		free(new_indexes);
		return errno;
	}

	for (int i = 0; i < card_list_len; i++)
		old_indexes[i] = i;

	for (int i = 0; i < card_list_len; i++)
	{
		// Randomly pick a value in old_indexes other than -1
		index = rand() % card_list_len;
		while (old_indexes[index] == -1)
			if (++index == card_list_len)
				index = 0;

		// Set new_indexes's next element to the value found before
		new_indexes[i] = old_indexes[index];

		// Set the value found before to -1 so it isn't picked again
		old_indexes[index] = -1;
	}

	card_t **temp_card_list;
	if ((temp_card_list = calloc(card_list_len, sizeof(card_t *))) == NULL)
	{
		free(old_indexes);
		free(new_indexes);
		return errno;
	}

	/*
	 * Change the order of cards in card_list by placing cards at their corresponding index in new_indexes
	 *
	 * (e.g. card at index 0 is placed at the index of value 0 in new_indexes)
	 */
	for (int i = 0; i < card_list_len; i++)
		temp_card_list[i] = card_list[new_indexes[i]];
	for (int i = 0; i < card_list_len; i++)
		card_list[i] = temp_card_list[i];

	// Free mem and return success
	free(temp_card_list);
	free(new_indexes);
	free(old_indexes);
	return 0;
}
