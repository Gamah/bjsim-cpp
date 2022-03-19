#pragma once
#include <vector>

class hand{
    public:
        std::vector<int> cards;
        int total = 0;
        int isPair = 0;
        bool isSoft = false;
        bool isSplit = false;
        bool isDoubled = false;
        bool isSurrendered = false;
        bool isInsured = false;
        int topCard = 0;
        int canSplit = 1;
        bool canDouble = false;  
        bool canSurrender = false;
        int trueCount = 0;
        int numCards = 0;

        void discard();
        void addCard(int cardIndex);
        void print();
};