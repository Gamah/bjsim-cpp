#include "include/utilities.h"
#include <iostream>
#include <algorithm>

#include <math.h>

void debugPrint(std::string string){
    if(config::debug){
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
    //used for deeper debugging
    //return "[" + deck(cardIndex) + face(cardIndex) + suit(cardIndex) + "]";
    return "[" + face(cardIndex) + "]";
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
    isSurrendered = 0;
    topCard = 0;
    canSplit = 0;
    canDouble = 0;
    canSurrender = rules::Surrender;
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
        isSoft = 1;
    }
    //if the hand was soft and goes over 21, subtract 10 and make it a hard hand
    if(total > 21 && isSoft == 1){
        total = total - 10;
        isSoft = 0;
    }
    //if this is the 2nd card, we need to check for pairs
    if (numCards == 2){
        canDouble = 1;
        //check if the card doubled matches the total plus the card for a pair
        //also check if a previous soft 11 (single ace) plus the incoming card (ace) match by adding up to 12
        if (total  == cardValue * 2 || (total == 12 && cardValue == 1)){
            isPair = cardValue;
            canSplit = 1;
        }
    }else{
        isPair = 0;
        canDouble = 0;
    }
    return;
}

void hand::print(){
    for(int x = 0; x < cards.size(); x++){
            std::cout << card::print(cards[x]);
        }
        std::cout << "\r\nTotal: " << total << " Doubled: " << isDoubled << " Split: " << isSplit << " Surrendered: " << isSurrendered << "\r\n";

}

//implement player funcitons
void player::addHand(hand hand){
    hands.push_back(hand);
}

player::player(){
    for(int x = 0;x < 15; x++){
        for(int y = 0; y < 5; y++){
            handResults[x][y] = 0;
        }
    }
}
void player::print(){
    for(hand h : hands){
        h.print();
    }
}

void  player::clearHands(){
    hands.clear();
}

void player::addResult(int trueCount, int handResult){
            if(trueCount < -7){
                trueCount = -7;
            }
            if(trueCount > 7){
                trueCount = 7;
            }
            //+7 to offset so that the 7 elements below are negative tc and 7 elements above are positive tc
            handResults[trueCount+7][handResult]++;
        }

//implement shoe funcitons
int shoe::getCard(){
    int newCard = cards.back();
    cards.pop_back();
    switch(card::value(newCard)){
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:{
            ++runningCount;
            break;
        }
        case 1:
        case 10:{
            --runningCount;
            break;
        }
        default:
            break;
    }   
    const int decksLeft = (((cards.size() - 1) / 52) + 1);
    trueCount = runningCount / decksLeft;
    return newCard;
}

void shoe::shuffleCards(){
    cards.clear();
    for(int x = 0; x < 52*6; x ++){
        cards.push_back(x);
    }
    
    std::shuffle(cards.begin(),cards.end(),config::mt);

    trueCount = 0;
    runningCount = 0;
}