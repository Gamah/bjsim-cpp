#pragma once
#include "config.h"
#include "card.h"

class hand{
    public:
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

        inline void discard(){
            total = 0;
            isPair = 0;
            isSoft = 0;
            isSplit = 0;
            isDoubled = 0;
            isSurrendered = 0;
            topCard = 0;
            canSplit = 0;
            canDouble = 0;
            canSurrender = false;
            numCards = 0;
        }

        inline void addCard(int cardIndex){
            int cardValue = card::value(cardIndex);
            topCard = cardIndex;
            numCards++;
            total = total + cardValue;
            if(total < 12 && cardValue == 1) [[unlikely]] {
                total = total + 10;
                isSoft = true;
            }
            if(total > 21 && isSoft) [[unlikely]] {
                total = total - 10;
                isSoft = false;
            }
            if(numCards == 2){
                if(!isSplit || config::rules::DAS){
                    canDouble = true;
                }
                if(!isSplit && config::rules::Surrender){
                    canSurrender = true;
                }else{
                    canSurrender = false;
                }
                if(total == cardValue * 2 || (total == 12 && cardValue == 1)){
                    isPair = cardValue;
                    canSplit = 1;
                }
            }else{
                isPair = 0;
                canDouble = false;
                canSurrender = false;
            }
        }

        void print();
};
