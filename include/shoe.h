#pragma once
#include <vector>
#include "xoshiro.h"
#include "card.h"
#include "config.h"

class shoe{
    public:
        std::vector<int> cards;
        int runningCount;
        int downCard;

        void shuffleCards(xoshiro256pp& rengine);

        inline void updateRunningCount(int cardValue){
            switch(cardValue){
                case 2: case 3: case 4: case 5: case 6:
                    ++runningCount; break;
                case 1: case 10:
                    --runningCount; break;
                default: break;
            }
        }

        inline int getCard(){
            int newCard = cards.back();
            cards.pop_back();
            updateRunningCount(card::value(newCard));
            return newCard;
        }

        inline int getDownCard(){
            downCard = cards.back();
            cards.pop_back();
            return downCard;
        }

        inline void flipDownCard(){
            updateRunningCount(card::value(downCard));
            downCard = 0;
        }

        inline float trueCount(){
            int quarterDecksLeft = ((int)cards.size() + 12) / 13;
            return (float)(runningCount * 4) / quarterDecksLeft;
        }
};
