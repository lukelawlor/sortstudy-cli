#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "card.h"

card_t **card_list;
int card_list_len = 0;

/*
 * int read_card(char *filename)
 *
 * reads a file and stores its contents into cards contained in card_list
 *
 * returns nonzero on file or memory allocation errors
 */
int read_card(char *filename)
{
	// Temp list used to store card data before it's transferred to card_list
	int temp_card_list_len, temp_card_list_size;
	card_t **temp_card_list, **temp_ptr;
	card_t *card;

	temp_card_list_size = CARD_ARRAY_ESTSIZE;
	if ((temp_card_list = calloc(temp_card_list_size, sizeof(card_t *))) == NULL)
	{
		perror("calloc");
		return 1;
	}

	// Vars for reading the card file
	bool front;
	
	// Buffer used to store characters read on each line; bp = buffer position
	char buffer[MAX_LINE_CHARS];
	int bp, c;

	front = true;
	temp_card_list_len = bp = 0;

	// Open card file
	FILE *cardfile;
	if ((cardfile = fopen(filename, "r")) == NULL)
	{
		perror("fopen");
		return 1;
	}

	// Read the card file
	do
	{
		if ((c = fgetc(cardfile)) == '\n')
		{
			// Null-terminate the buffer so it can safely be copied into strings
			buffer[bp++] = '\0';

			// Check if the front or back of the card is being read
			if (front)
			{
				// Allocate mem for a new card
				if ((card = malloc(sizeof(card_t))) == NULL)
				{
					perror("malloc");
					goto read_card_error;
				}
				if ((card->front = calloc(bp, sizeof(char))) == NULL)
				{
					perror("malloc");
					free(card);
					goto read_card_error;
				}
				if ((card->back = calloc(bp, sizeof(char))) == NULL)
				{
					perror("malloc");
					free(card->front);
					free(card);
					goto read_card_error;
				}

				// Copy buffer into the front text of the card
				strncpy(card->front, buffer, bp);

				// Store a pointer to the card in the list if there's enough space in the array,
				// if not, resize the array
				if (temp_card_list_len >= temp_card_list_size)
				{
					temp_card_list_size += CARD_ARRAY_ESTSIZE;
					if ((temp_ptr = reallocarray(temp_card_list, temp_card_list_size, sizeof(card_t *))) == NULL)
					{
						perror("reallocarray");
						goto read_card_error;
					}
					temp_card_list = temp_ptr;
				}
				temp_card_list[temp_card_list_len] = card;
			}
			else
			{
				// Copy buffer into the back text of the card
				strncpy(card->back, buffer, bp);
				temp_card_list_len++;
			}

			// Invert the value of front and reset the buffer position
			front = front ? false : true;
			bp = 0;
		}
		else	// Collect characters in buffer
			buffer[bp++] = c;
	} while (c != EOF);

	if (fclose(cardfile) == EOF)
	{
		perror("fclose");
		goto read_card_error;
	}

	// Resize the list to fit the actual number of card pointers it contains
	if ((temp_ptr = reallocarray(temp_card_list, temp_card_list_len, sizeof(card_t *))) == NULL)
	{
		perror("reallocarray");
		goto read_card_error;
	}
	temp_card_list = temp_ptr;

	// Resize card_list, then copy temp_card_list's contents into it
	card_list_len = temp_card_list_len;
	if ((temp_ptr = reallocarray(card_list, card_list_len, sizeof(card_t *))) == NULL)
	{
		perror("reallocarray");
		goto read_card_error;
	}
	card_list = temp_ptr;

	for (int i = 0; i < temp_card_list_len; i++)
		card_list[i] = temp_card_list[i];
	free(temp_card_list);

	return 0;
	
	// Free temp list & its contents on random errors
	read_card_error:
	for (int i = 0; i < temp_card_list_len; i++)
		free(temp_card_list[i]);
	free(temp_card_list);
	return 1;
}
