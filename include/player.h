#pragma once
#include "hand.h"
#include <vector>
#include <iostream>

class player{
    public:
        std::string name;
        int strategy;
        std::vector<hand> hands;
        // 65 quarter-TC buckets (-8.0 to +8.0 in 0.25 steps) x 10 result types
        long handResults[65][10];

        player(std::string name, int strategy);
        void addHand(hand& hand);
        void print();
        void clearHands();
        void addResult(float trueCount, int handResult);
        void printResults();
};