#pragma once
#include "utilities.h"
#include <map>

class strategies{
    public:
        decisions dealerH17(hand hand);
        decisions dealerS17(hand hand);
        decisions playerBasic(int total, int upCard, int isPair, int isSoft, int canSplit, bool canDouble, bool canSurrender);      
        decisions playerH17Deviations(hand& hand, int trueCount, int upCard);  
};