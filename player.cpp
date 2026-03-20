#include <iostream>
#include <string>
#include <cmath>
#include "include/player.h"
#include "include/utilities.h"

player::player(std::string name, int strategy){
    player::name = name;
    player::strategy = strategy;
    for(int x = 0; x < 65; x++){
        for(int y = 0; y < 10; y++){
            handResults[x][y] = 0;
        }
    }
}

void player::addHand(hand& hand){
    hands.push_back(hand);
}

void player::print(){
    for(hand h : hands){
        h.print();
    }
}

void player::clearHands(){
    hands.clear();
    hands.reserve(config::rules::maxSplit);
}

// Quarter-TC bucketing: index = floor(tc * 4) + 32, clamped to [0, 64]
// Index 0 = TC -8.0, index 32 = TC 0.0, index 64 = TC +8.0
void player::addResult(float trueCount, int handResult){
    int idx = (int)std::floor(trueCount * 4.0f) + 32;
    if(idx < 0) idx = 0;
    if(idx > 64) idx = 64;
    handResults[idx][handResult]++;
}

void player::printResults(){
    std::cout << "tc,doublelose,lose,surrender,insurancelose,insurancewin,push,win,blackjack,doublewin,roundsplayed";
    for(int x = 0; x < 65; x++){
        float tc = (x - 32) / 4.0f;
        std::cout << "\r\n" << tc << ",";
        for(int y = 0; y < 10; y++){
            std::cout << handResults[x][y] << ",";
        }
    }
    std::cout << "\r\n\r\n" << std::endl;
}
