#include "strategies.h"

namespace strategies{
    enum class decisions{HIT, STAND, SPLIT, DOUBLE, SURRENDER};

    class dealerH17{
        decisions play(gameObjects::hand){
            if(hand.total < 17 || (hand.total == 17 && hand.isSoft == 1)){
                return utilities::decisions.HIT;
            }else{
                return utilities::decisions.STAND;
            }
        }
    };

    class dealerS17{
        decisions play(gameObjects::hand){
            if(hand.total < 17){
                return utilities::decisions.HIT;
            }else{
                return utilities::decisions.STAND;
            }
        }
    };

}