#pragma once
#include "utilities.h"
#include <map>

class strategies{
    public:
        decisions dealerH17(hand hand);
        decisions dealerS17(hand hand);
        decisions playerBasic(hand& hand, int upCard);      
        decisions playerH17Deviations(hand& hand, int upCard, int trueCount, int runningCount);  
};