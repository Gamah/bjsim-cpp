#include "include/card.h"
#include "include/hand.h"
#include <iostream>

void hand::print(){
    std::cout << "\r\nTotal: " << total << " Soft: " << isSoft << " Doubled: " << isDoubled << " Split: " << isSplit << " Surrendered: " << isSurrendered << "\r\n";
}
