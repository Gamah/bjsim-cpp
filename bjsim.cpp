#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include "include/utilities.h"
#include "include/strategies.h"

// random generator function:
int randomizer (int i) { return std::rand()%i;};

int main() {

    //initialize shoe
    //TODO: break this out of main so it can be threaded
    std::srand ( unsigned ( std::time(0) ) );
    std::vector<int> shoe;

    //setup dealer and players
    strategies strategy;
    decisions decision;
    hand dealer;    
    std::vector<player> players;
    players.push_back(player());
    
    
    for(int x = 0; x < (52*6); x++){
        shoe.push_back(x);
    }

    // using built-in random generator:
    std::random_shuffle( shoe.begin(), shoe.end());

    //start a round of bj
    while(shoe.size() > 76){
        for(player& p : players){
            p.addHand(hand());
        }
        int upCard = 0;        
        
        //deal 2 cards to everyone
        for(int x = 0;x<2;x++){
            dealer.addCard(shoe.back());
            if(x%2 == 1){
                upCard = card::value(shoe.back());
            }
            shoe.pop_back();
            for(player& p : players){
                for(hand& h : p.hands){
                    h.addCard(shoe.back());
                    shoe.pop_back();
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
                    decision = strategy.playerBasic(h.total,upCard,h.isPair,h.isSoft,h.canSplit,h.canDouble,h.canSurrender);
                    while(decision != decisions::STAND){
                        //don't play split aces
                        if(h.canSplit == -1){
                            decision = decisions::STAND;
                        }else{
                            switch(decision){
                                case decisions::HIT : {
                                    h.addCard(shoe.back());
                                    shoe.pop_back();
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
                                    h.addCard(shoe.back());
                                    shoe.pop_back();
                                    //deal to the new hand
                                    newhand.addCard(shoe.back());
                                    shoe.pop_back();
                                    //if the hands are aces or player has 4 hands (lengh of hands + new hand not yet added) then they can't resplit
                                    if(p.hands.size() + 1 == rules::maxSplit){
                                        h.canSplit = 0;
                                        newhand.canSplit = 0;
                                    }
                                    //this is a dumb hack to indicate split aces because i can't read the future to prevent playing or resplitting a split ace on a future hand
                                    if(card::value(h.topCard) == 1){
                                        h.canSplit = -1;
                                        newhand.canSplit = -1;
                                    }else{
                                        decision = strategy.playerBasic(h.total,upCard,h.isPair,h.isSoft,h.canSplit,h.canDouble,h.canSurrender);
                                    }
                                    //mark hands slpit and add to player's hands
                                    h.isSplit = 1;
                                    newhand.isSplit =1;
                                    p.addHand(newhand);
                                    break;
                                }
                                case decisions::DOUBLE :{
                                    //TODO: double the bet...
                                    h.addCard(shoe.back());
                                    shoe.pop_back();
                                    decision = decisions::STAND;
                                    break;
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

                dealer.addCard(shoe.back());
                shoe.pop_back();
                decision = strategy.dealerH17(dealer);
            }
        }
            

        //debug print cards
        if(config::debug){

            debugPrint("Dealer: \r\n");
            dealer.print();
            debugPrint("Players: \r\n");

            //check for player wins
            //escape comparisons if bust
            for(player& p : players){
                for(hand& h : p.hands){
                    h.print();
                    if(h.total <= 21){
                        if(h.total > dealer.total || dealer.total > 21){
                            debugPrint("Player won!\r\n");
                        }
                        if(h.total == dealer.total){
                            debugPrint("Player push\r\n");
                        }
                        if(h.total < dealer.total && dealer.total <= 21){
                            debugPrint("Player lost\r\n");
                        }
                    }else{
                        debugPrint("Player BUST!\r\n");
                    }
                }   
            }
            debugPrint("\r\n\r\n");
        }

        dealer.discard();
        for(player& p : players){
            p.clearHands();
        }
    }
    return 0;
}