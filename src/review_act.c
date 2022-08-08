#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

#include "card.h"
#include "review_act.h"

bool cards_flipped = false;

static int delete_marked_cards(void);

/*
 * swaps the back text of cards with the front text
 */
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
	cardstate_t *temp_review_list;
	if ((temp_card_list = calloc(card_list_len, sizeof(card_t *))) == NULL)
	{
		free(old_indexes);
		free(new_indexes);
		return errno;
	}
	if ((temp_review_list = calloc(card_list_len, sizeof(cardstate_t))) == NULL)
	{
		free(old_indexes);
		free(new_indexes);
		free(temp_card_list);
		return errno;
	}

	// Change the order of cards in card_list by placing cards at their corresponding index in new_indexes
	// (e.g. card at index 0 is placed at the index of value 0 in new_indexes)
	for (int i = 0; i < card_list_len; i++)
		temp_card_list[i] = card_list[new_indexes[i]];
	for (int i = 0; i < card_list_len; i++)
		card_list[i] = temp_card_list[i];

	// Change the order of bools in review_list by placing bools at their corresponding index in new_indexes
	for (int i = 0; i < card_list_len; i++)
		temp_review_list[i] = review_list[new_indexes[i]];
	for (int i = 0; i < card_list_len; i++)
		review_list[i] = temp_review_list[i];
	
	// Free mem and return success
	free(temp_review_list);
	free(temp_card_list);
	free(new_indexes);
	free(old_indexes);
	return 0;
}

/*
 * Deletes a single card at the specified index in card_list
 *
 * returns errno on error
 */
int delete_card(int index)
{
	review_list[index] = TO_DELETE;
	return delete_marked_cards();
}

/*
 * Deletes every card in card_list with a state of TO_DELETE
 *
 * returns errno on error
 */
static int delete_marked_cards(void)
{
	// Allocate new mem for card_list
	card_t **new_list;
	int new_len;

	new_len = card_list_len;
	for (int i = 0; i < card_list_len; i++)
		if (review_list[i] == TO_DELETE)
			new_len--;
	
	if ((new_list = calloc(new_len, sizeof(card_t *))) == NULL)
		return errno;
	
	// Free memory of delete cards and set their indexes in card_list to NULL
	for (int i = 0; i < card_list_len; i++)
	{
		if (review_list[i] == TO_DELETE)
		{
			free(card_list[i]->back);
			free(card_list[i]->front);
			free(card_list[i]);
			card_list[i] = NULL;
		}
	}
	
	// Add cards from card_list to new_list
	
	// Position in new_list
	int np;

	np = 0;
	for (int i = 0; i < card_list_len; i++)
	{
		if (card_list[i] != NULL)
			new_list[np++] = card_list[i];
	}

	// Free data left in card_list and set it to new_list
	free(card_list);
	card_list = new_list;

	return 0;
}
