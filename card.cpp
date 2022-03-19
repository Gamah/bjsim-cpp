#include "include/card.h"
#include "include/utilities.h"
#include <iostream>
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
