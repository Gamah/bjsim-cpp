#include "utilities.h"
#include "strategies.h"

decisions strategies::dealerH17(int total, bool isSoft){
    if(total < 17 || (total == 17 && isSoft == 1)){
        return decisions::HIT;
    }else{
        return decisions::STAND;
    }
}

decisions strategies::dealerS17(int total){
    if(total < 17){
        return decisions::HIT;
    }else{
        return decisions::STAND;
    }
}

