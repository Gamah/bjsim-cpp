#pragma once
#include "utilities.h"

class strategies{
    public:
        decisions dealerH17(int total, bool isSoft);
        decisions dealerS17(int total);
        decisions playerBasic(int total, bool isSoft);
};