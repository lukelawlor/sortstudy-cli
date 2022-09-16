/*
 * review_act.h
 *
 * This file contains function prototypes for functions defined in review_act.c
 */

#ifndef	REVIEW_ACT_H
#define	REVIEW_ACT_H

// True if the deck of cards has been flipped
extern bool cards_flipped;

// Swap front and back text of cards in card_list
void flip_cards(void);

// Shuffle cards in card_list
int shuffle_cards(void);

#endif
