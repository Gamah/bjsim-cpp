#pragma once
#include <string>
#include <vector>

namespace config{
    static const bool debug = false;
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
        int topCard = 0;
        int canSplit = 1;
        bool canDouble = false;  
        bool canSurrender = false;

        //TODO: figure out constructors
        //hand(double bet = 0,int total = 0,int isPair = 0,bool isSoft = 0,bool isSplit = 0,bool isDoubled = 0,int canSplit = rules().maxSplit,bool canDouble = 0,bool canSurrender = 0);
        void discard();
        void addCard(int cardIndex);
        void print();
};

class player{
    public:
        std::vector<hand> hands;
        double bankroll;
        double betUnit;
        int betMultiplier;

        //TODO: figure out constructors
        //player(double bankroll = 0, double betUnit = 0, int betMultiplier = 0);
        void addHand(hand hand);
        void print();
        void clearHands();
};

class shoe{
    std::vector<int> cards;
    int runningCount;
    int trueCount;
};