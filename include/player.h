#pragma once
#include "hand.h"
#include <vector>
#include <iostream>

class player{
    public:
        std::string name;
        std::vector<hand> hands;
        //+/-7 true count totals for Losses, Pushes, Surrenders(lost insurance), Wins, and BlackJacks
        long handResults[15][10];
        
        player(std::string name);
        void addHand(hand& hand);
        void print();
        void clearHands();
        void addResult(int trueCount, int handResult);
        void printResults();
};