#include "include/shoe.h"
#include "include/card.h"
#include "include/utilities.h"
#include <iostream>
#include <random>
#include <algorithm>
//implement shoe funcitons
void shoe::updateRunningCount(int cardValue){
    switch(cardValue){
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
    return;
}

int shoe::getCard(){
    int newCard = cards.back();
    cards.pop_back();
    updateRunningCount(card::value(newCard));
    return newCard;
}

int shoe::getDownCard(){
    downCard = cards.back();
    cards.pop_back();
    return(downCard);
}

void shoe::flipDownCard(){
    updateRunningCount(card::value(downCard));
    downCard = 0;
    return;
}



int shoe::trueCount(){
    int decksLeft = (((cards.size() - 1) / 52) + 1);
    return(runningCount / decksLeft);
}

void shoe::shuffleCards(std::mt19937& rengine){
    cards.clear();
    for(int x = 0; x < 52*config::rules::numDecks; x ++){
        cards.push_back(x);
    }   
    
    std::shuffle(cards.begin(),cards.end(),rengine);

    runningCount = 0;
}