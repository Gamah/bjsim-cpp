#include "include/card.h"
#include "include/hand.h"
#include "include/utilities.h"
#include <iostream>

//implement hand functions
void hand::discard(){
    cards.clear();
    total = 0;
    isPair = 0;
    isSoft = 0;  
    isSplit = 0;
    isDoubled = 0;
    isSurrendered = 0;
    topCard = 0;
    canSplit = 0;
    canDouble = 0;
    canSurrender = false;
    numCards = 0;
    return;
}

void hand::addCard(int cardIndex){
    //TODO: fix ace and 21 totaling, blackjacks are technically a soft 11...
    //add the card to the list of cards
    int cardValue = card::value(cardIndex);
    topCard = cardIndex;
    if(config::debug){
        cards.push_back(cardIndex);
    }
    numCards++;
    total = total + cardValue;
    //if the current card is an ace and the hand total is less than 12, lets add 11 instead of 1
    if(total < 12 && cardValue == 1){
        total = total + 10;
        isSoft = true;
    }
    //if the hand was soft and goes over 21, subtract 10 and make it a hard hand
    if(total > 21 && isSoft){
        total = total - 10;
        isSoft = false;
    }
    //if this is the 2nd card, we need to check for pairs
    if (numCards == 2){
        if(!isSplit || rules::DAS){
            canDouble = true;
        }
        if(!isSplit && rules::Surrender){
            canSurrender = true;
        }else{
            canSurrender = false;
        }
        //check if the card doubled matches the total plus the card for a pair
        //also check if a previous soft 11 (single ace) plus the incoming card (ace) match by adding up to 12
        if (total  == cardValue * 2 || (total == 12 && cardValue == 1)){
            isPair = cardValue;
            canSplit = 1;
        }
    }else{
        isPair = 0;
        canDouble = false;
        canSurrender = false;
    }
    return;
}

void hand::print(){
    for(int x = 0; x < cards.size(); x++){
            std::cout << card::print(cards[x]);
        }
        std::cout << "\r\nTotal: " << total << " Soft: " << isSoft  << " Doubled: " << isDoubled << " Split: " << isSplit << " Surrendered: " << isSurrendered << "\r\n";

}