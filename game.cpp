#include "include/strategies.h"
#include "include/utilities.h"
#include "include/hand.h"
#include "include/player.h"
#include "include/shoe.h"
#include "include/card.h"
#include "include/game.h"
#include <mutex>

void game::runGame(std::mt19937 rengine, long& shoesPlayed, std::mutex& processResults, std::vector<player>& playersPlayed) {
    //initialize sgame
    shoe shoe;
    strategies strategy;
    decisions decision;
    hand dealer;    
    
    std::vector<player> players;
    //TODO: implement mulitple players...
    players.push_back(player());

    //TODO: break this out of main so it can be threaded
    for(int x = 0;x<config::settings::numShoes;x++){

        shoe.shuffleCards(rengine);
        debugPrint("Shuffle!");

        if(config::settings::debug){
            for(int & c :shoe.cards){
                std::cout << card::print(c) << ",";
            }
            std::cout << "\r\n";
        }
        //burn a card...
        shoe.getCard();


        //start a round of bj
        while(shoe.cards.size() >= config::rules::deckPen){
            for(player& p : players){
                hand newHand;
                newHand.discard();
                //imagine the bets are set out here, this is the TC the hand will belong to in the final out.
                newHand.trueCount = shoe.trueCount();
                p.addResult(newHand.trueCount,handResults::roudsplayed);
                p.addHand(newHand);
            }
            int upCard = 0;
            debugPrint("RunningCount: " + std::to_string(shoe.runningCount));
            debugPrint("TrueCount: " + std::to_string(shoe.trueCount()));
            debugPrint("Cards Dealt: " + std::to_string(312 - shoe.cards.size()));
            debugPrint("Cards Left: " + std::to_string(shoe.cards.size()) + "\r\n");

            //deal 2 cards to everyone
            for(int x = 0;x<2;x++){
                for(player& p : players){
                    for(hand& h : p.hands){
                        h.addCard(shoe.getCard());
                    }
                }
                if(x%2 == 1){
                    int newCard = shoe.getCard();
                    dealer.addCard(newCard);
                    upCard = card::value(newCard);
                }else{
                    dealer.addCard(shoe.getDownCard());
                }

            }

            //check for dealer Ace up
            if(upCard == 1){
                if(shoe.trueCount() >= 3){
                    for(player& p : players){
                        for(hand& h : p.hands){
                            h.isInsured = true;
                        }
                    }
                }
            }
            //play the round if dealer doesn't have blackjack
            if(dealer.total != 21){
                bool dealerPlays = false;
                //player turns
                for(player& p : players){
                    for(hand& h : p.hands){            
                        decision = strategy.playerDeviations(h,upCard,shoe.trueCount(),shoe.runningCount);
                        //decision = strategy.playerBasic(h,upCard);
                        while(decision != decisions::STAND){
                            //don't play split aces
                            if(h.canSplit == -1){
                                //unless it's a pair of aces and resplitting is allowed and max hands is not hit
                                if(card::value(h.topCard) == 1 && config::rules::RSA && p.hands.size() + 1 <= config::rules::maxSplit){
                                    h.canSplit = 1;
                                    decision = decisions::SPLIT;
                                }else{
                                    decision = decisions::STAND;
                                }
                            }else{
                                switch(decision){
                                    case decisions::HIT : {
                                        h.addCard(shoe.getCard());
                                        decision = strategy.playerDeviations(h,upCard,shoe.trueCount(),shoe.runningCount);
                                        //decision = strategy.playerBasic(h,upCard);
                                        break;
                                    }
                                    case decisions::SPLIT : {
                                        //new hand object for after dealing with this hand...
                                        hand newhand;
                                        newhand.discard();
                                        int topCard = h.topCard;
                                        //pull card off the top if debugging is on
                                        if(config::settings::debug){
                                            h.cards.pop_back();
                                        }                  
                                        h.numCards--;
                                        h.isPair = 0;
                                        //mark hands slpit
                                        h.isSplit = true;
                                        newhand.isSplit = true;
                                        h.canSurrender = false;
                                        newhand.canSurrender = false;
                                        newhand.trueCount = h.trueCount;

                                        //halve the hand total
                                        h.total = h.total - card::value(topCard);
                                        //put the top card into the new hand            
                                        newhand.addCard(topCard);
                                        //deal to the current hand
                                        h.addCard(shoe.getCard());
                                        //deal to the new hand
                                        newhand.addCard(shoe.getCard());
                                        //if the hands are aces or player has 4 hands (lengh of hands + new hand not yet added) then they can't resplit
                                        if(p.hands.size() + 1 == config::rules::maxSplit){
                                            h.canSplit = 0;
                                            newhand.canSplit = 0;
                                        }
                                        //this is a dumb hack to indicate split aces because i can't read the future to prevent playing or resplitting a split ace on a future hand
                                        if(card::value(topCard) == 1){
                                            h.canSplit = -1;
                                            newhand.canSplit = -1;
                                            if(card::value(h.topCard) == 1 && config::rules::RSA && p.hands.size() + 1 <= config::rules::maxSplit){
                                                decision = decisions::SPLIT;
                                            }else{
                                                decision = decisions::STAND;
                                            }
                                        }else{
                                            decision = strategy.playerDeviations(h,upCard,shoe.trueCount(),shoe.runningCount);
                                            //decision = strategy.playerBasic(h,upCard);
                                        }
                                        //add to player's hands
                                        p.addHand(newhand);
                                        break;
                                    }
                                    case decisions::DOUBLE :{
                                        h.addCard(shoe.getCard());
                                        h.isDoubled = true;
                                        decision = decisions::STAND;
                                        break;
                                    }
                                    case decisions::SURRENDER : {
                                        h.isSurrendered = true;
                                        decision = decisions::STAND;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                //dealer turn
                for(player& p :players){
                    if(dealerPlays){
                        break;
                    }
                    for(hand& h : p.hands){
                        if(h.total <=21){
                            dealerPlays = true;
                            break;
                        }
                    }
                }
                //flip over the down card even if the dealer doesn't have to play
                shoe.flipDownCard();
                if(dealerPlays){
                    decision = strategy.dealer(dealer);
                    while(decision != decisions::STAND){
                        if(decision == decisions::HIT){
                            dealer.addCard(shoe.getCard());
                            decision = strategy.dealer(dealer);
                        }
                    }
                }
            }else{
                //flip the card to 'show' a dealer blackjack
                //TODO: just move this outside of the current scope... that should be safe?
                shoe.flipDownCard();
            }


            //Determine outcome of hands...
            debugPrint("Dealer");
            if(config::settings::debug){
                dealer.print();
            }
            for(player& p : players){
                for(hand& h : p.hands){
                    debugPrint("Player");
                    if(config::settings::debug){
                        h.print();
                    }
                    //if the hand was insured, it's either a push or we take the insurance bet.
                    if(h.isInsured){
                        if(dealer.total == 21 && dealer.numCards == 2){
                            p.addResult(h.trueCount,handResults::insurancewin);
                            debugPrint("Insurance Win");
                            continue;
                        }else{
                            p.addResult(h.trueCount,handResults::insurancelose);
                            debugPrint("Insurance Lose");
                        }
                    }

                    //if dealer has blackjack push or take bets and exit loop
                    if(dealer.total == 21 && dealer.numCards == 2){
                        if(h.total == 21 && h.numCards == 2){
                            p.addResult(h.trueCount,handResults::push);
                            debugPrint("BLackjack Push");
                        }else{
                            p.addResult(h.trueCount,handResults::lose);
                            debugPrint("Blackjack Lose");
                        }
                        continue;
                    }

                    //if player has blackjack pay and exit loop
                    if(h.total == 21 && h.numCards ==2 && h.isSplit == false){
                        p.addResult(h.trueCount,handResults::blackjack);
                        debugPrint("Blackjack Win");
                        continue;
                    }

                    //if player has surendered, take bet and exit loop
                    if(h.isSurrendered){
                        p.addResult(h.trueCount,handResults::surrender);
                        debugPrint("Surrender Lose");
                        continue;
                    }

                    //check for busts...
                    if(h.total > 21){
                        if(h.isDoubled){
                            p.addResult(h.trueCount,handResults::doublelose);
                            debugPrint("Player double bust");
                        }else{
                            p.addResult(h.trueCount,handResults::lose);
                            debugPrint("Player bust");
                        }
                        continue;
                    }
                    if(dealer.total > 21){
                        if(h.isDoubled){
                            p.addResult(h.trueCount,handResults::doublewin);
                            debugPrint("Dealer bust double win");

                        }else{
                            p.addResult(h.trueCount,handResults::win);
                            debugPrint("Dealer bust win");
                        }
                        continue;
                    }

                    //compare the hands for win/loss/push
                    if(h.total <= 21 && dealer.total <= 21){
                        if(h.total > dealer.total){
                            if(h.isDoubled){
                                p.addResult(h.trueCount,handResults::doublewin);
                                debugPrint("Player double win");
                            }else{
                                p.addResult(h.trueCount,handResults::win);
                                debugPrint("Player win");
                            }
                            continue;
                        }
                        if(h.total < dealer.total){
                            if(h.isDoubled){
                                p.addResult(h.trueCount,handResults::doublelose);
                                debugPrint("Player double lose");
                            }else{
                                p.addResult(h.trueCount,handResults::lose);
                                debugPrint("Player lose");
                            }
                            continue;
                        }
                        if(h.total == dealer.total){
                            p.addResult(h.trueCount,handResults::push);
                            debugPrint("Player push");
                            continue;
                        }
                    }

                }  
            }

            dealer.discard();
            for(player& p : players){
                p.clearHands();
            }
            if(config::settings::debug){
                std::cout << "\r\n\r\n";
            }

            shoesPlayed = x + 1;
        }
    }   
        processResults.lock();
        for(player& p : players){
            playersPlayed.push_back(p);
        }
        processResults.unlock();
        return;
}   