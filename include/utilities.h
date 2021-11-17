#pragma once
#include <string>
#include <vector>
#include <random>

namespace config{
    static const bool debug = false;
    static std::mt19937_64 mt(time(nullptr));
};

void debugPrint(std::string string);

enum class decisions{HIT, STAND, DOUBLE, SPLIT, SURRENDER};
namespace rules{
    static const bool H17 = 0;
    static const bool DAS = 1;
    static const bool RSA = 0;
    static const bool Surrender = 0;
    static const int maxSplit = 4;
};

namespace handResults{
    static const int lose = 0;
    static const int surrender = 1;
    static const int push = 2;
    static const int win = 3;
    static const int blackjack = 4;
}

namespace card{    
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

class player{
    public:
        std::vector<hand> hands;
        //+/-7 true count totals for Losses, Pushes, Surrenders(lost insurance), Wins, and BlackJacks
        unsigned int handResults[15][5];
    
        void addHand(hand hand);
        void print();
        void clearHands();
        void addResult(int trueCount, int handResult);
        player();
};

class shoe{
    public:
        std::vector<int> cards;
        int runningCount;
        int trueCount;

        void shuffleCards();
        int getCard();
};