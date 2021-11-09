#include "utilities.h"
#include "strategies.h"

decision strategies::dealerH17(int total, bool isSoft){
    if(total > 17 || (total == 17 && isSoft == 1)){
        return decision.HIT;
    }else{
        return decision.STAND;
    }
}

decision strategies::dealerS17(int total){
    if(total > 17){
        return decision.HIT;
    }else{
        return decision.STAND;
    }
}

