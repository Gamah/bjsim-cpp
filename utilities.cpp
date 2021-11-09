#include <string>
#include <list>
#include <vector>
#include "utilities.h"

//implement card functions
int card::value(int cardIndex){
    return values[cardIndex%13];   
}

std::string card::face(int cardIndex){
    return faces[cardIndex % 13];
}

std::string card::suit(int cardIndex){
    return suits[(cardIndex % 52) / 13];
}

std::string card::deck(int cardIndex){
    return decks[cardIndex / 52];
}

std::string card::print(int cardIndex){
    return "[" + deck(cardIndex) + face(cardIndex) + suit(cardIndex) + "]";
}


//implement hand functions
void hand::discard(){
    cards.clear();
    bet = 0;
    total = 0;
    isPair = 0;
    isSoft = 0;  
    isSplit = 0;
    isDoubled = 0;
    canSplit = 0;
    canDouble = 0;
};

void hand::addCard(int cardIndex){
    //add the card to the list of cards
    int cardValue = card().value(cardIndex);
    cards.push_back(cardIndex);
    //if this is the 2nd card, we need to check for pairs
    if (cards.size() == 2){
        //check if the card coubled matches the total plus the card for a double
        //also check if a previous soft 11 (single ace) plus the incoming card (ace) match by adding up to 12
        if (total + cardValue == cardValue * 2 || (total + cardValue == 12 && cardValue == 1)){
            isPair = 1;
            //if they're aces, it's a soft hand
            if (cardValue == 1){
                isSoft = 1;
            }else{
                isSoft = 0;
            }

        }   
    total = total + cardValue;
    }else{
        isPair = 0;
        //if the current card is an ace and the hand total is less than 12, lets add 11 instead of 1
        total = total + cardValue;
        if(total < 12 && cardValue == 1){
            total = total + 10;
            isSoft = 1;
        }
        if(total > 21 && isSoft == 1){
            total = total - 10;
            isSoft = 0;
        }
    }
    return;
}