#include <iostream>
#include <string>
#include <vector>
#include "include/utilities.h"
#include "include/strategies.h"

int main() {
    //initialize sgame
    shoe shoe;
    strategies strategy;
    decisions decision;
    hand dealer;    
    std::vector<player> players;
    //TODO: implement mulitple players...
    players.push_back(player());

    
    //TODO: break this out of main so it can be threaded
    for(int x = 0;x<1000;x++){

    shoe.shuffleCards();
    debugPrint("Shuffle!");

    //start a round of bj
    while(shoe.cards.size() > 78){
        
        for(player& p : players){
            hand newHand;
            newHand.discard();
            p.addHand(newHand);
        }
        int upCard = 0;        

        //deal 2 cards to everyone
        for(int x = 0;x<2;x++){
            int newCard = shoe.getCard();
            dealer.addCard(newCard);
            if(x%2 == 1){
                upCard = card::value(newCard);
            }
            for(player& p : players){
                for(hand& h : p.hands){
                    h.addCard(shoe.getCard());
                    h.trueCount = shoe.trueCount;
                    
                }
            }
        }

        //check for dealer Ace up
        if(upCard == 1){
            if(shoe.trueCount >= 3){
                for(player& p : players){
                    for(hand& h : p.hands){
                        //h.isInsured = true;
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
                    //TODO: switch this back to just passing the hand in so hand's true count can be updated.
                    h.trueCount = shoe.trueCount;
                    decision = strategy.playerBasic(h.total,upCard,h.isPair,h.isSoft,h.canSplit,h.canDouble,h.canSurrender);
                    while(decision != decisions::STAND){
                        //don't play split aces
                        if(h.canSplit == -1){
                            decision = decisions::STAND;
                        }else{
                            h.trueCount = shoe.trueCount;
                            switch(decision){
                                case decisions::HIT : {
                                    h.addCard(shoe.getCard());
                                    h.trueCount = shoe.trueCount;
                                    decision = strategy.playerBasic(h.total,upCard,h.isPair,h.isSoft,h.canSplit,h.canDouble,h.canSurrender);
                                    break;
                                }
                                case decisions::SPLIT : {
                                    //new hand object for after dealing with this hand...
                                    hand newhand;
                                    newhand.discard();
                                    int topCard = h.topCard;
                                    //pull card off the top if debugging is on
                                    if(config::debug){
                                        h.cards.pop_back();
                                    }                  
                                    h.numCards--;
                                    h.isPair = 0;
                                    //halve the hand total
                                    h.total = h.total - card::value(topCard);
                                    //put the top card into the new hand            
                                    newhand.addCard(topCard);
                                    //deal to the current hand
                                    h.addCard(shoe.getCard());
                                    //deal to the new hand
                                    newhand.addCard(shoe.getCard());
                                    //if the hands are aces or player has 4 hands (lengh of hands + new hand not yet added) then they can't resplit
                                    if(p.hands.size() + 1 >= rules::maxSplit){
                                        for(hand& h2 : p.hands){
                                            h2.canSplit = 0;
                                        }
                                        h.canSplit = 0;
                                        newhand.canSplit = 0;
                                    }
                                    //this is a dumb hack to indicate split aces because i can't read the future to prevent playing or resplitting a split ace on a future hand
                                    if(card::value(topCard) == 1){
                                        h.canSplit = -1;
                                        newhand.canSplit = -1;
                                        decision = decisions::STAND;
                                    }else{
                                        decision = strategy.playerBasic(h.total,upCard,h.isPair,h.isSoft,h.canSplit,h.canDouble,h.canSurrender);
                                    }
                                    //mark hands slpit and add to player's hands
                                    h.isSplit = true;
                                    newhand.isSplit = true;
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
            //if players hands are all bust or surrender, dealer doesn't need to play
            for(player& p :players){
                if(dealerPlays){
                    break;
                }
                for(hand& h : p.hands){
                    if(h.total <=21 && !h.isSurrendered){
                        dealerPlays = true;
                        break;
                    }
                }
            }
            //dealer turn
            if(dealerPlays){
                decision = strategy.dealerS17(dealer);
                while(decision != decisions::STAND){
                    if(decision == decisions::HIT){
                        dealer.addCard(shoe.getCard());
                        decision = strategy.dealerS17(dealer);
                    }
                }
            }
        }


        //Determine outcome of hands...
        debugPrint("Dealer");
        if(config::debug){
            dealer.print();
        }
        for(player& p : players){
            for(hand& h : p.hands){
                debugPrint("Player");
                if(config::debug){
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
                if(h.total == 21 and h.numCards ==2 && h.isSplit == false){
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
                    }
                    if(h.total < dealer.total){
                        if(h.isDoubled){
                            p.addResult(h.trueCount,handResults::doublelose);
                            debugPrint("Player double lose");
                        }else{
                            p.addResult(h.trueCount,handResults::lose);
                            debugPrint("Player lose");
                        }
                    }
                    if(h.total == dealer.total){
                        p.addResult(h.trueCount,handResults::push);
                        debugPrint("Player push");
                    }
                }

            }   
        }
        
        dealer.discard();
        for(player& p : players){
            p.clearHands();
        }
        if(config::debug){
            std::cout << "\r\n\r\n";
        }
    }
    }
    for(player& p : players){
        std::cout << "count,doublelose,lose,surrender,insurancelose,insurancewin,push,win,blackjack,doublewin";
        for(int x = -7; x <= 7;x++){
            std::cout << "\r\n" << x << ",";
            for(int y = 0; y < 9; y++){
                std::cout << p.handResults[x+7][y] << ",";
            }
        }
        std::cout << "\r\n\r\n";
    }
    return 0;
}