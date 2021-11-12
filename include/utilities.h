#pragma once
#include <string>
#include <vector>

struct config{
    bool debug = 1;
};

void debugPrint(std::string string);

enum class decisions{HIT, STAND, DOUBLE, SPLIT, SURRENDER};
struct rules{
    bool H17 = 0;
    bool DAS = 1;
    bool RSA = 0;
    bool Surrender = 0;
    int maxSplit = 4;
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
        double bet;
        int total;
        int isPair;
        bool isSoft;
        bool isSplit;
        bool isDoubled;
        int canSplit;
        bool canDouble; 
        bool canSurrender;

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
};
