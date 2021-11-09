#include "strategies.h"

namespace gameObjects{

    class card{
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
            gameObjects::card card;
            int total;
            bool isPair;
            bool isSoft;
            bool isSplit;
            bool isDoubled;
            int bet;
            int canSplit;
            bool canDouble;

            void discard();

            void addCard(int cardIndex);
    };

    class player{
        public:
            std::vector<gameObjects::hand> hands;
            std::string name;
            int bankroll;
            strategies::dealerH17 strategy;
            int betUnit;
            int betMultiplier;
    };


    
}