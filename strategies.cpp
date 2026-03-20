#include "include/strategies.h"
#include "include/utilities.h"
#include "include/hand.h"
#include "include/card.h"
#include <stdexcept>
#include <iostream>


//implement strategy functions
decisions strategies::play(hand& hand, int& upcard, shoe& shoe, int strategy){
    switch(strategy){
        case 0:
            return dealer(hand);
            break;
        case 1:
            return playerBasic(hand,upcard);
            break;
        case 2:
            return playerDeviations(hand,upcard,shoe.trueCount(), shoe.runningCount);
            break;
    }

    throw std::invalid_argument("Invalid strategy passed to play function");
}

decisions strategies::dealer(hand& hand){
    if(hand.total < 17 || (hand.total == 17 && hand.isSoft && config::rules::H17)){
        return decisions::HIT;
    }else{
        return decisions::STAND;
    }
}

decisions strategies::playerBasic(hand& hand,int upCard){
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
                switch(upCard){
                    case 2:
                    case 3:
                        if(config::rules::DAS){
                            return decisions::SPLIT;
                        }
                        break;
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        return decisions::SPLIT;
                }
                break;
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
                        if(config::rules::DAS){
                            return decisions::SPLIT;
                        }
                        break;
                }
                break;
            case 6:
                switch(upCard){
                    case 2:
                        if(config::rules::DAS){
                            return decisions::SPLIT;;
                        }
                        break;
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
    
    if(hand.isSoft){
        switch(hand.total){
            case 20:
            case 21:
                return decisions::STAND;
            case 19:
                switch(upCard){
                    case 6:
                        if(hand.canDouble){
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
                        if(hand.canDouble){
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
                        if(hand.canDouble){
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
                        if(hand.canDouble){
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
                        if(hand.canDouble){
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
    if(hand.canSurrender){
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
            break;
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
            if(hand.canDouble){
                return decisions::DOUBLE;
            }
            default:
                return decisions::HIT;
            break;
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
                    if(hand.canDouble){
                        return decisions::DOUBLE;
                    }
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
                    if(hand.canDouble){
                        return decisions::DOUBLE;
                    }
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

decisions strategies::playerDeviations(hand& hand, int upCard, float trueCount, int runningCount){
    const uint32_t mask = config::rules::deviationMask;

    //Pair Splitting
    if(hand.isPair == 10){
        switch(upCard){
            case 4:{
                if((mask & dev::SPLIT_10_VS_4) && trueCount >= 6 && hand.canSplit == 1){
                    debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Split 10s TC 6+");
                    return decisions::SPLIT;
                }
                break;
            }
            case 5:{
                if((mask & dev::SPLIT_10_VS_5) && trueCount >= 5 && hand.canSplit == 1){
                    debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Split 10s TC 5+");
                    return decisions::SPLIT;
                }
                break;
            }
            case 6:{
                if((mask & dev::SPLIT_10_VS_6) && trueCount >= 4 && hand.canSplit == 1){
                    debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Split 10s TC 4+");
                    return decisions::SPLIT;
                }
            }
        }
    }

    //Soft Totals
    if(hand.isSoft && hand.total == 19){
        if((mask & dev::SOFT19_D4) && upCard == 4 && trueCount >= 3 && hand.canDouble){
            debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Double soft 19 vs 4 TC3+");
            return decisions::DOUBLE;
        }
        if((mask & dev::SOFT19_D5) && upCard == 5 && trueCount >= 1 && hand.canDouble){
            debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Double soft 19 vs 5 tc1+");
            return decisions::DOUBLE;
        }
        if((mask & dev::SOFT19_S6) && upCard == 6 && runningCount < 0){
            debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Soft 19 stand vs 6 negative count");
            return decisions::STAND;
        }
    }

    if((mask & dev::SOFT17_D2) && hand.isSoft && hand.total == 17 && upCard == 2 && trueCount >= 1 && hand.canDouble){
        debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Double soft 17 vs 2 TC1+");
        return decisions::DOUBLE;
    }

    //Hard Totals
    if(!hand.isSoft || (hand.isPair != 0 && hand.canSplit == 0)){
        if(hand.total == 16){
            if((mask & dev::H16_S9) && upCard == 9 && trueCount >= 4 && !hand.canSurrender){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Hard 16 stands vs 9 TC4+");
                return decisions::STAND;
            }
            if((mask & dev::H16_S10) && upCard == 10 && runningCount >= 0 && !hand.canSurrender){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Hard 16 stands vs 10 positive count");
                return decisions::STAND;
            }
            if((mask & dev::H16_SA) && upCard == 1 && trueCount >= 3 && !hand.canSurrender){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Hard 16 stands vs Ace TC3+");
                return decisions::STAND;
            }
        }
        if(hand.total == 15){
            if((mask & dev::H15_S10) && upCard == 10 && trueCount >= 4 && !hand.canSurrender){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Hard 15 stands vs 10 TC4+");
                return decisions::STAND;
            }
            if((mask & dev::H15_SA) && upCard == 1 && trueCount >= 5 && !hand.canSurrender){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Hard 15 stands vs Ace TC5+");
                return decisions::STAND;
            }
        }
        if((mask & dev::H13_H2) && hand.total == 13 && upCard == 2 && trueCount <= -1){
            debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Hard 13 hits vs 2 TC -1 and less");
            return decisions::HIT;
        }
        if(hand.total == 12){
            if((mask & dev::H12_S2) && upCard == 2 && trueCount >= 3){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Hard 12 stands vs 2 TC 3+");
                return decisions::STAND;
            }
            if((mask & dev::H12_S3) && upCard == 3 && trueCount >= 2){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Hard 12 stands vs 3 TC2+");
                return decisions::STAND;
            }
            if((mask & dev::H12_H4) && upCard == 4 && runningCount < 0){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " Hard 12 hits vs 4 negative count");
                return decisions::HIT;
            }
        }
        if(hand.total == 10){
            if((mask & dev::H10_D10) && upCard == 10 && trueCount >= 4 && hand.canDouble){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 10 doubles vs 10 TC4+");
                return decisions::DOUBLE;
            }
            if((mask & dev::H10_DA) && upCard == 1 && trueCount >= 3 && hand.canDouble){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 10 doubles vs Ace TC3+");
                return decisions::DOUBLE;
            }
        }
        if(hand.total == 9){
            if((mask & dev::H9_D2) && upCard == 2 && trueCount >= 1 && hand.canDouble){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 9 doubles vs 2 TC1+");
                return decisions::DOUBLE;
            }
            if((mask & dev::H9_D7) && upCard == 7 && trueCount >= 3 && hand.canDouble){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 9 doubles vs 7 TC3+");
                return decisions::DOUBLE;
            }
        }
        if((mask & dev::H8_D6) && hand.total == 8 && upCard == 6 && trueCount >= 2 && hand.canDouble){
            debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 8 doubles vs 6 TC2+");
            return decisions::DOUBLE;
        }

        //Surrender deviations
        if(hand.canSurrender){
            if((mask & dev::H17_SUR_A) && config::rules::H17 && hand.total == 17 && upCard == 1){
                debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 17 always surrenders vs Ace in H17");
                return decisions::SURRENDER;
            }
            if(hand.total == 16){
                if((mask & dev::H16_SUR_8) && upCard == 8 && trueCount >= 4){
                    debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 16 surrenders vs 8 TC4+");
                    return decisions::SURRENDER;
                }
                if((mask & dev::H16_H9) && upCard == 9 && trueCount <= -1){
                    debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 16 vs 9 hits TC -1 and less");
                    return decisions::HIT;
                }
                if((mask & dev::H16_SUR_TA) && config::rules::H17 && (upCard == 10 || upCard == 1)){
                    debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 16 always surrender vs 10 and ace in H17");
                    return decisions::SURRENDER;
                }
            }
            if(hand.total == 15){
                if((mask & dev::H15_SUR_9) && upCard == 9 && trueCount >= 2){
                    debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 15 surrenders vs 9 at TC2+");
                    return decisions::SURRENDER;
                }
                if((mask & dev::H15_H10) && upCard == 10 && runningCount < 0){
                    debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 15 hits vs 10 any negative count");
                    return decisions::HIT;
                }
                if((mask & dev::H15_SUR_A) && upCard == 1 && trueCount <= -1){
                    debugPrint("DEVIATION: TC:" + std::to_string(trueCount) + " 15 surrenders ace TC-1 and above");
                    return decisions::SURRENDER;
                }
            }
        }
    }
    return(playerBasic(hand,upCard));
}
