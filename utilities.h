#pragma once
#include <string>
#include <vector>

class card{
    private:
        int values[13] = {1,2,3,4,5,6,7,8,9,10,10,10,10};
        std::string suits[4] = {"H","D","S","C"};
        std::string faces[13] = {"A","2","3","4","5","6","7","8","9","T","J","Q","K"};
        std::string decks[8] = {"1","2","3","4","5","6","7","8"};

    public: 
        int value(int cardIndex);
        std::string face(int cardIndex);
        std::string suit(int cardIndex);
        std::string deck(int cardIndex);
        std::string print(int cardIndex);
};

class hand{
    public:
        std::vector<int> cards;
        double bet = 0;
        int total = 0;
        int isPair = 0;
        bool isSoft = 0;
        bool isSplit = 0;
        bool isDoubled = 0;
        int canSplit = 0;
        bool canDouble = 0; 
        bool canSurrender = 0;

        void discard();

        void addCard(int cardIndex);

        void print();
};

class player{
    public:
        std::vector<hand> hands;
        double bankroll;
        double betUnit;
        double betMultiplier;

        void addHand(hand hand);
};

enum class decisions{HIT, STAND, DOUBLE, SPLIT, SURRENDER};


class rules{
    bool H17 = 0;
    bool DAS = 1;
    bool RSA = 0;
    bool Surrender = 0;
};