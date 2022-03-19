#include <iostream>
#include "include/player.h"
#include "include/utilities.h"
//implement player funcitons
void player::addHand(hand& hand){
    hands.push_back(hand);
}

player::player(){  
    //initialize array... is this necessary?
    for(int x = 0;x < 15; x++){
        for(int y = 0; y < 10; y++){
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
    hands.reserve(rules::maxSplit);
}

void player::addResult(int trueCount, int handResult){
    //clamp truecount to no more than 7 in either direction
    if(trueCount < -7){
        trueCount = -7;
    }
    if(trueCount > 7){
        trueCount = 7;
    }
    //+7 to offset so that the 7 elements below are negative tc and 7 elements above are positive tc
    handResults[trueCount+7][handResult]++;
}

void player::printResults(){
    std::cout << "count,doublelose,lose,surrender,insurancelose,insurancewin,push,win,blackjack,doublewin,roundsplayed";
            for(int x = -7; x <= 7;x++){
                std::cout << "\r\n" << x << ",";
                for(int y = 0; y < 10; y++){
                    std::cout << handResults[x+7][y] << ",";
                }
            }
            std::cout << "\r\n\r\n" << std::endl;
            
}