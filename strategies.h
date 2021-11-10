#pragma once
#include "utilities.h"

class strategies{
    decision dealerH17(int total, bool isSoft);

    decision dealerS17(int total);

    decision playerBasic(int total, bool isSoft);
};