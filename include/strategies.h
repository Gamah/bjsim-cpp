#pragma once
#include "utilities.h"
#include "hand.h"
#include "shoe.h"

class strategies{
    public:
        decisions play(hand& hand, int& upcard, shoe& shoe, int strategy);
        decisions dealer(hand& hand);
        decisions playerBasic(hand& hand, int upCard);      
        decisions playerDeviations(hand& hand, int upCard, int trueCount, int runningCount);  
};