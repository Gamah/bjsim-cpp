#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <random>
#include <cstdlib>
#include "include/utilities.h"
#include "include/strategies.h"

// random generator function:

int main() {
    std::srand(time(nullptr));
    //initialize shoe
    //TODO: break this out of main so it can be threaded
    std::srand ( unsigned ( std::time(0) ) );
    shoe shoe;

    //setup dealer and players
    strategies strategy;
    decisions decision;
    hand dealer;    
    std::vector<player> players;
    players.push_back(player());

    for(int x = 0;x<1000000;x++){
    // using built-in random generator:
    shoe.shuffleCards();

    //start a round of bj
    while(shoe.cards.size() > 76){
        
        for(player& p : players){
            p.addHand(hand());
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
                }
            }

        }

        //check for dealer Ace up
        //TODO: Offer insurance
        //debug print if ace up
        if(upCard == 1){
            debugPrint("Dealer has ace!\r\n");
        }
        //check for dealer blackjack
        //TODO: take bets or push hands, clean up hands and deal next round
        if(dealer.total == 21 || (dealer.total == 11 && dealer.isSoft)){
            debugPrint("Dealer has blackjack!\r\n");
        //if the dealer didn't have blackjack, play the round...

        }else{

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
                                    decision = strategy.playerBasic(h.total,upCard,h.isPair,h.isSoft,h.canSplit,h.canDouble,h.canSurrender);
                                    break;
                                }
                                case decisions::SPLIT : {
                                    //new hand object for after dealing with this hand...
                                    //TODO: implement bets
                                    hand newhand = hand();
                                    //pull card off the top if debugging is on
                                    if(config::debug){
                                        h.cards.pop_back();
                                    }                    
                                    //halve the hand total
                                    h.total == h.total / 2;
                                    //put the top card into the new hand            
                                    newhand.addCard(h.topCard);
                                    //deal to the current hand
                                    h.addCard(shoe.getCard());
                                    //deal to the new hand
                                    newhand.addCard(shoe.getCard());
                                    //if the hands are aces or player has 4 hands (lengh of hands + new hand not yet added) then they can't resplit
                                    if(p.hands.size() + 1 == rules::maxSplit){
                                        h.canSplit = 0;
                                        newhand.canSplit = 0;
                                    }
                                    //this is a dumb hack to indicate split aces because i can't read the future to prevent playing or resplitting a split ace on a future hand
                                    if(card::value(h.topCard) == 1){
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
                                    decision = decisions::STAND;
                                    break;
                                }
                                case decisions::DOUBLE :{
                                    //TODO: double the bet...
                                    h.addCard(shoe.getCard());
                                    h.isDoubled = true;
                                    decision = decisions::STAND;
                                    break;
                                }
                                case decisions::SURRENDER : {
                                    //TODO: take half the bet.
                                    h.isSurrendered = true;
                                    decision = decisions::STAND;
                                }
                            }
                        }
                    }
                }
            }
        }
        //dealer turn
        decision = strategy.dealerH17(dealer);
        while(decision != decisions::STAND){
            if(decision == decisions::HIT){

                dealer.addCard(shoe.getCard());
                decision = strategy.dealerH17(dealer);
            }
        }


        //debug print cards
        if(config::debug){
            dealer.print();
        }

        debugPrint("Dealer: \r\n");
        debugPrint("Players: \r\n");
        //check for player wins
        //escape comparisons if bust
        for(player& p : players){
            for(hand& h : p.hands){
                if(config::debug){
                    h.print();
                }
                if(h.total <= 21 || !h.isSurrendered){
                    if(h.total > dealer.total || dealer.total > 21){
                        p.addResult(h.trueCount,handResults::win);
                        debugPrint("Player won!\r\n");
                    }
                    if(h.total == dealer.total){
                        p.addResult(h.trueCount,handResults::push);
                        debugPrint("Player push\r\n");
                    }
                    if(h.total < dealer.total && dealer.total <= 21){
                        p.addResult(h.trueCount,handResults::lose);
                        debugPrint("Player lost\r\n");
                    }
                }else{
                    if(h.isSurrendered){
                        p.addResult(h.trueCount,handResults::surrender);
                    }else{
                        p.addResult(h.trueCount,handResults::lose);
                    }
                    debugPrint("Player BUST/Surrender!\r\n");
                }
            }   
        }
        debugPrint("\r\n\r\n");

        dealer.discard();
        for(player& p : players){
            p.clearHands();
        }
    }
    }
    for(player& p : players){
        std::cout << "lose      surr        push        win     bj";
        for(int x = -7; x < 7;x++){
            std::cout << x << ":\r\n";
            for(int y = 0; y < 5; y++){
                std::cout << "    " << p.handResults[x][y] << "    ";
            }
        }
    }
    return 0;
}