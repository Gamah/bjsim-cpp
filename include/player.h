#pragma once
#include "hand.h"
#include <list>
#include <iostream>

class player{
    public:
        std::list<hand> hands;
        //+/-7 true count totals for Losses, Pushes, Surrenders(lost insurance), Wins, and BlackJacks
        unsigned int handResults[15][10];
    
        void addHand(hand& hand);
        void print();
        void clearHands();
        void addResult(int trueCount, int handResult);
        void printResults();
        player();
};