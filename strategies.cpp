#include "utilities.h"
#include "strategies.h"
#include <stdexcept>


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

decisions strategies::playerBasic(hand hand, int upCard){
    //Always Split Aces & 8’s
    //Never Split 10’s and 5’s
    //Split 2, 3 & 7 on 2 through 7
    //Split 4’s against 5 & 6
    //Split 6’s on 2 through 6
    //Split 9’s against 2 through 9, except 7

    if(hand.canSplit == 1){
        switch(hand.isPair){
            case 1:
            case 8:
                return decisions::SPLIT;
            case 2:
            case 3:
            case 7:
                switch(upCard){
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        return decisions::SPLIT;
                }
                break;
            case 4:
                switch(upCard){
                    case 5:
                    case 6:
                        return decisions::SPLIT;
                }
                break;
            case 6:
                switch(upCard){
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                        return decisions::SPLIT;
                }
                break;
            case 9:
                switch(upCard){
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 8:
                    case 9:
                        return decisions::SPLIT;
                }
                break;
        }
    }


    //Soft 21 and 20 always stand
    //Soft 19 (Ace, 8) doubles against 6 otherwise it stands
    //Soft 18 (Ace, 7) doubles against 2-6, stands against 7 and 8, hits against 9, 10, Ace. If it can’t double against 2-6, it stands
    //Soft 17 (A, 6) doubles against 3-6, otherwise it hits
    //Soft 16 and Soft 15 (A, 5 and A, 4) doubles against 4-6 otherwise it hits
    //Soft 14 and soft 13 (Ace, 3 and ace, 2) doubles against 5 and 6 otherwise it hits
    
    if(hand.isSoft == 1){
        switch(hand.total){
            case 20:
            case 21:
                return decisions::STAND;
            case 19:
                switch(upCard){
                    case 6:
                        if(hand.canDouble ==1){
                            return decisions::DOUBLE;
                        }
                    default:
                        return decisions::STAND;
                }
                break;
            case 18:
                switch(upCard){
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                        if(hand.canDouble ==1){
                            return decisions::DOUBLE;
                        }else{
                            return decisions::STAND;
                        }
                    case 7:
                    case 8:
                        return decisions::STAND;
                    case 9:
                    case 10:
                    case 1:
                        return decisions::HIT;
                }
                break;
            case 17:
                switch(upCard){
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                        if(hand.canDouble ==1){
                            return decisions::DOUBLE;
                        }
                    default:
                        return decisions::HIT;
                }
                break;
            case 16:
            case 15:
                switch(upCard){
                    case 4:
                    case 5:
                    case 6:
                        if(hand.canDouble ==1){
                            return decisions::DOUBLE;
                        }
                    default:
                        return decisions::HIT;
                }
                break;
            case 14:
            case 13:
                switch(upCard){
                    case 5:
                    case 6:
                        if(hand.canDouble ==1){
                            return decisions::DOUBLE;
                        }
                    default:
                        return decisions::HIT;
                }
                break;
        }
    }


    //Surrender 16 against 9 – Ace
    //Surrender 15 against a 10
    if(hand.canSurrender ==1){
        switch(hand.total){
            case 16:
                switch(upCard){
                    case 9:
                    case 10:
                    case 1:
                        return decisions::SURRENDER;
                }
                break;
            case 15:
                if(upCard == 10){
                    return decisions::SURRENDER;
                }
        }
    }


    //Hard 17 and above will always stand
    //Hard 13 through 16 will stand against 2-6, hit against 7-ace
    //Hard 12 will stand against 4-6, hit against everything else
    //Hard 11 will always double down
    //Hard 10 will double against 2-9
    //Hard 9 will double against 3-6
    //Hard 8 and below will always hit

    if(hand.total >= 17){
        return decisions::STAND;
    }
    switch(hand.total){
        case 16:
        case 15:
        case 14:
        case 13:
            switch(upCard){
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                    return decisions::STAND;
                default:
                    return decisions::HIT;
            }
            break;
        case 12:
            switch(upCard){
                case 4:
                case 5:
                case 6:
                    return decisions::STAND;
                default:
                    return decisions::HIT;
            }
            break;
        case 11:
            return decisions::DOUBLE;
        case 10:
            switch(upCard){
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                    return decisions::DOUBLE;
                default:
                    return decisions::HIT;
            }
            break;
        case 9:
            switch(upCard){
                case 3:
                case 4:
                case 5:
                case 6:
                    return decisions::DOUBLE;
                default:
                    return decisions::HIT;
            }
            break;
    }
    if(hand.total <= 8){
        return decisions::HIT;
    }

    throw std::invalid_argument("Invalid hand passed to Basic Strategy function");
    
}
