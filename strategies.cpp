#include "utilities.h"
#include "strategies.h"


//implement strategy functions
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

decisions strategies::playerBasic(hand hand){
    //Always Split Aces & 8’s
    //Never Split 10’s and 5’s
    //Split 2, 3 & 7 on 2 through 7
    //Split 4’s against 5 & 6
    //Split 6’s on 2 through 6
    //Split 9’s against 2 through 9, except 7
    //Soft 21 and 20 always stand
    //Soft 19 (Ace, 8) doubles against 6 otherwise it stands
    //Soft 18 (Ace, 7) doubles against 2-6, stands against 7 and 8, hits against 9, 10, Ace. If it can’t double against 2-6, it stands
    //Soft 17 (A, 6) doubles against 3-6, otherwise it hits
    //Soft 16 and Soft 15 (A, 5 and A, 4) doubles against 4-6 otherwise it hits
    //Soft 14 and soft 13 (Ace, 3 and ace, 2) doubles against 5 and 6 otherwise it hits
    //Surrender 16 against 9 – Ace
    //Surrender 15 against a 10
    //Hard 17 and above will always stand
    //Hard 13 through 16 will stand against 2-6, hit against 7-ace
    //Hard 12 will stand against 4-6, hit against everything else
    //Hard 11 will always double down
    //Hard 10 will double against 2-9
    //Hard 9 will double against 3-6
    //Hard 8 and below will always hit
    return decisions::STAND;
}
