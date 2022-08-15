/*
 * card.c
 *
 * This file contains variables with card data and functions used to load and manipulate decks of cards
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "util.h"
#include "card.h"

// Array holding card structs
card_t **card_list;
int card_list_len;

/*
 * reads a file containing card text and stores its contents into cards contained in card_list, replacing the previous contents of card_list if successful, and resizing review_list to hold the maximum amount of cards needed to review
 *
 * returns errno on file or memory allocation errors
 */
int read_deck(char **filenames, int filecount)
{
	// Temp array used to store card data before it's transferred to card_list
	// temp_card_list_len 	refers to the number of cards in the array
	// temp_card_list_size	refers to the number of elements the array can hold
	int temp_card_list_len, temp_card_list_size;
	card_t **temp_card_list;
	card_t *card;

	card = NULL;
	temp_card_list_size = CARD_ARRAY_ESTSIZE;
	if ((temp_card_list = calloc(temp_card_list_size, sizeof(card_t *))) == NULL)
	{
		perror("calloc");
		return errno;
	}

	// Vars for reading the card file
	bool front;
	
	// Buffer used to store characters read on each line; bp = buffer position
	char buffer[MAX_LINE_CHARS];
	int bp, c;

	front = true;
	temp_card_list_len = bp = 0;

	// Open card files and read them
	FILE *cardfile;

	for (int filenum = 0; filenum < filecount; filenum++)
	{
		if ((cardfile = fopen(filenames[filenum], "r")) == NULL)
		{
			perror("fopen");
			return errno;
		}

		while ((c = fgetc(cardfile)) != EOF)
		{
			if (c == '\n' || bp == MAX_LINE_CHARS - 1)
			{
				// Null-terminate the buffer so it can safely be copied into strings
				buffer[bp++] = '\0';

				// Check if the front or back of the card is being read
				if (front)
				{
					// Allocate mem for a new card, its front string, and its state
					if ((card = malloc(sizeof(card_t))) == NULL)
					{
						perror("malloc");
						goto read_deck_error;
					}
					if ((card->front = calloc(bp, sizeof(char))) == NULL)
					{
						perror("calloc");
						free(card);
						goto read_deck_error;
					}

					// Initializing state
					card->state = DO_REVIEW;

					// Copy buffer into the front string of the card
					strncpy(card->front, buffer, bp);

				}
				else
				{
					// Allocate mem for the back string of the card
					if ((card->back = calloc(bp, sizeof(char))) == NULL)
					{
						perror("calloc");
						free(card->front);
						free(card);
						goto read_deck_error;
					}
					
					// Copy buffer into the back text of the card
					strncpy(card->back, buffer, bp);

					// Store a pointer to the card in the list if there's enough space in the array,
					// if not, resize the array
					if (temp_card_list_len == temp_card_list_size)
					{
						temp_card_list_size += CARD_ARRAY_ESTSIZE;
						if ((temp_card_list = reallocarray(temp_card_list, temp_card_list_size, sizeof(card_t *))) == NULL)
						{
							perror("reallocarray");
							free_card(card);
							goto read_deck_error;
						}
					}
					temp_card_list[temp_card_list_len++] = card;
					card = NULL;
				}

				// Invert the value of front and reset the buffer position
				front = front ? false : true;
				bp = 0;
			}
			else
			{
				// Collect characters in buffer

				// Handle escape sequences
				if (c == '\\')
				{
					switch (c = fgetc(cardfile))
					{
						case 'n':
							c = '\n';
							break;
						case '\n':
							// Backslash was placed at the end of a line
							buffer[bp++] = '\\';
							ungetc(c, cardfile);
							continue;
					}
					
				}

				buffer[bp++] = c;
			}

		}

		if (fclose(cardfile) == EOF)
		{
			perror("fclose");
			goto read_deck_error;
		}

		// Handle reading errors
		if (card != NULL)
		{
			// A card's front has been read, but not its back
			fprintf(stderr, "sortstudycli: no back text found for a card in file \"%s\"\n", filenames[filenum]);
			free(card->front);
			free(card);
			errno = EIO;
			goto read_deck_error;
		}
	}

	if (temp_card_list_len == 0)
	{
		// No cards were fully read
		fprintf(stderr, "sortstudycli: no cards found in file(s)\n");
		free(temp_card_list);
		return EIO;
	}

	// Resize temp_card_list to fit the actual number of card pointers it contains
	if ((temp_card_list = reallocarray(temp_card_list, temp_card_list_len, sizeof(card_t *))) == NULL)
	{
		perror("reallocarray");
		goto read_deck_error;
	}

	// Resize review_list 

	// Alloc new mem for card_list; if successful, free everything in card_list and set its value
	// to the new pointer
	card_t **temp_card_list_ptr;
	if ((temp_card_list_ptr = calloc(temp_card_list_len, sizeof(card_t *))) == NULL)
	{
		perror("calloc");
		goto read_deck_error;
	}
	free_card_list(card_list, card_list_len);
	card_list = temp_card_list_ptr;

	// Copy the elements of temp_card_list into card_list, then set card_list_len
	for (int i = 0; i < temp_card_list_len; i++)
		card_list[i] = temp_card_list[i];
	card_list_len = temp_card_list_len;

	free(temp_card_list);
	return 0;
	
	// Free temp list & its contents on random errors
	read_deck_error:

	free_card_list(temp_card_list, temp_card_list_len);
	return errno;
}

/*
 * frees a card pointer (type card_t *)
 */
void free_card(card_t *card)
{
	free(card->front);
	free(card->back);
	free(card);
}

/*
 * frees a card list (type card_t **) and all of its elements
 */
void free_card_list(card_t **list, int len)
{
	for (int i = 0; i < len; i++)
		free_card(list[i]);
	free(list);
}

/*
 * Deletes every card in card_list with a state of TO_DELETE
 *
 * returns errno on error
 */
int delete_marked_cards(void)
{
	// Allocate new mem for card_list and review_list
	card_t **new_card_list;
	int new_len;

	new_len = card_list_len;
	for (int i = 0; i < card_list_len; i++)
		if (card_list[i]->state == TO_DELETE)
			new_len--;
	
	if ((new_card_list = calloc(new_len, sizeof(card_t *))) == NULL)
		return errno;

	// Position in new_card_list
	int np;

	// Add card pointers to new_card_list and free cards marked for deletion
	np = 0;
	for (int i = 0; i < card_list_len; i++)
	{
		if (card_list[i]->state != TO_DELETE)
		{
			// Card isn't marked for deletion, add its pointer to new_card_list
			new_card_list[np++] = card_list[i];
		}
		else
		{
			// Card is marked for deletion, free it
			free_card(card_list[i]);
		}
	}

	// Free card_list itself and set it to new_card_list
	free(card_list);
	card_list = new_card_list;
	card_list_len = new_len;

	return 0;
}
