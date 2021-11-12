#include "include/utilities.h"
#include <iostream>

void debugPrint(std::string string){
    if(config().debug){
        std::cout << string;
    }
    return;
}

static constexpr int values[13] = {1,2,3,4,5,6,7,8,9,10,10,10,10};
static const std::string suits[4] = {"H","D","S","C"};
static const std::string faces[13] = {"A","2","3","4","5","6","7","8","9","T","J","Q","K"};
static const std::string decks[8] = {"1","2","3","4","5","6","7","8"};

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
    canSurrender = 0;
    return;
}

void hand::addCard(int cardIndex){
    //TODO: fix ace and 21 totaling
    //add the card to the list of cards
    int cardValue = card::value(cardIndex);
    if(config().debug){
        cards.push_back(cardIndex);
    }
    //if this is the 2nd card, we need to check for pairs
    if (cards.size() == 2){
        //check if the card doubled matches the total plus the card for a pair
        //also check if a previous soft 11 (single ace) plus the incoming card (ace) match by adding up to 12
        if (total + cardValue == cardValue * 2 || (total + cardValue == 12 && cardValue == 1)){
            isPair = cardValue;
        }
    }else{
        isPair = 0;
    }

    total = total + cardValue;
    //if the current card is an ace and the hand total is less than 12, lets add 11 instead of 1
    if(total < 12 && cardValue == 1){
        total = total + 10;
        isSoft = 1;
    }
    //if the hand was soft and goes over 21, subtract 10 and make it a hard hand
    if(total > 21 && isSoft == 1){
        total = total - 10;
        isSoft = 0;
    }
    return;
}

void hand::print(){
    for(int x = 0; x < cards.size(); x++){
            std::cout << card::print(cards[x]);
        }
        std::cout << "\r\n Total: " << total << " Soft: " << isSoft << " Pair: " << isPair << "\r\n";

}

//implement player funcitons
void player::addHand(hand hand){
    hands.push_back(hand);
}