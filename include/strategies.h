#pragma once
#include "utilities.h"
#include <map>

class strategies{
    public:
        decisions dealer(hand hand);
        decisions playerBasic(hand& hand, int upCard);      
        decisions playerDeviations(hand& hand, int upCard, int trueCount, int runningCount);  
};