#include "utilities.h"
#include "strategies.h"

decisions strategies::dealerH17(hand hand){
    if(hand.total < 17 || (hand.total == 17 && hand.isSoft == 1)){
        return decisions::HIT;
    }else{
        return decisions::STAND;
    }
}

decisions strategies::dealerS17(hand hand){
    if(hand.total < 17){
        return decisions::HIT;
    }else{
        return decisions::STAND;
    }
}

