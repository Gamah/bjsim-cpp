#pragma once
#include <string>
#include <vector>
#include <random>
#include <list>

namespace config{
    static const bool debug = false;
    static const int numThreads = 4;
    static std::vector<std::mt19937> rengines;
    static const int numShoes = 500000;
};

void debugPrint(std::string string);

enum class decisions{HIT, STAND, DOUBLE, SPLIT, SURRENDER};
namespace rules{
    static const bool H17 = true;
    static const bool DAS = true;
    static const bool RSA = false;
    static const bool Surrender = false;
    static const int maxSplit = 4;
    static const int deckPen = 78;
};

namespace handResults{
    static const int doublelose = 0;
    static const int lose = 1;
    static const int surrender = 2;
    static const int insurancelose = 3;
    static const int insurancewin = 4;
    static const int push = 5;
    static const int win = 6;
    static const int blackjack = 7;
    static const int doublewin = 8;
    static const int roudsplayed = 9;
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

class shoe{
    public:
        std::vector<int> cards;
        int runningCount;
        int downCard;

        void shuffleCards(std::mt19937 rengine);
        void updateRunningCount(int cardValue);
        int getCard();
        int getDownCard();
        void flipDownCard();
        int trueCount();
};