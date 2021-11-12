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
    int numRounds;
    for(int x = 0;x < 1000000; x++){
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
    players[0].addHand(hand());

    
    for(int x = 0; x < (52*6); x++){
        shoe.push_back(x);
    }

    // using built-in random generator:
    std::random_shuffle( shoe.begin(), shoe.end());
    while(shoe.size() > 76){

        numRounds++;
        int upCard = 0;        
        
        //deal 2 cards to everyone
        for(int x = 0;x<2;x++){
            dealer.addCard(shoe.back());
            if(x%2 == 1){
                upCard = card::value(shoe.back());
            }
            shoe.pop_back();
            players[0].hands[0].addCard(shoe.back());
            shoe.pop_back();
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
            decision = strategy.playerBasic(players[0].hands[0].total,upCard,players[0].hands[0].isPair,players[0].hands[0].isSoft,players[0].hands[0].canSplit,players[0].hands[0].canDouble,players[0].hands[0].canSurrender);
            while(decision != decisions::STAND){
                if(decision == decisions::HIT|| decision == decisions::DOUBLE || decision == decisions::SPLIT){
                    players[0].hands[0].addCard(shoe.back());
                    shoe.pop_back();
                    decision = strategy.playerBasic(players[0].hands[0].total,upCard,players[0].hands[0].isPair,players[0].hands[0].isSoft,players[0].hands[0].canSplit,players[0].hands[0].canDouble,players[0].hands[0].canSurrender);
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
            

            //check for player wins
            //escape comparisons if bust
            if(players[0].hands[0].total <= 21){
                if(players[0].hands[0].total > dealer.total || dealer.total > 21){
                    debugPrint("Player won!\r\n");
                }
                if(players[0].hands[0].total == dealer.total){
                    debugPrint("Player push\r\n");
                }
                if(players[0].hands[0].total < dealer.total && dealer.total <= 21){
                    debugPrint("Player lost\r\n");
                }
            }else{
                debugPrint("Player BUST!\r\n");
            }
        }
        //debug print cards
        if(config().debug){
            debugPrint("Dealer: ");
            dealer.print();
            debugPrint("Player: ");
            players[0].hands[0].print();
            debugPrint("Cards left: " + std::to_string(shoe.size()) + "\r\n \r\n");
        }
        
        dealer.discard();
        players[0].hands[0].discard();
        
    }   

    }
    std::cout << numRounds << "\r\n";
    return 0;
}