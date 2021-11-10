#pragma once
#include "utilities.h"

class strategies{
    public:
        decisions dealerH17(hand hand);
        decisions dealerS17(hand hand);
        decisions playerBasic(hand hand);
};