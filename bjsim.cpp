#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include "utilities.h"
#include "strategies.h"

// random generator function:
int randomizer (int i) { return std::rand()%i;};

void test(){
    strategies strategy;
    card card;
    std::cout << "Hard Totals:\r\n";
    std::cout << "    2 3 4 5 6 7 8 9 T J Q K A";
    for(int x=5; x<22;x++){
        std::cout << "\r\n" << x << " ";
        if(x < 10){std::cout << " ";}
        for(int y = 2;y < 14;y++){
            hand hand;
            hand.total = x;
            hand.isSoft = 0;
            hand.isPair = 0;
            hand.canDouble = 1;
            hand.canSurrender = 1;
            switch(strategy.playerBasic(hand,y)){
                case decisions::HIT:
                    std::cout << " H";
                    break;
                case decisions::STAND:
                    std::cout << " S";
                    break;
                case decisions::DOUBLE:
                    std::cout << " D";
                    break;
                case decisions::SPLIT:
                    std::cout << " Q";
                    break;
                case decisions::SURRENDER:
                    std::cout << " X";
                    break;
            }
        }
        hand hand;
        hand.total = x;
        hand.isSoft = 0;
        hand.isPair = 0;
        hand.canDouble = 1;
        hand.canSurrender = 1;
        switch(strategy.playerBasic(hand,1)){
            case decisions::HIT:
                std::cout << " H";
                break;
            case decisions::STAND:
                std::cout << " S";
                break;
            case decisions::DOUBLE:
                std::cout << " D";
                break;
            case decisions::SPLIT:
                std::cout << " Q";
                break;
            case decisions::SURRENDER:
                std::cout << " X";
                break;
        }
    }
    
    std::cout << "\r\n\r\n";

    std::cout << "Soft Totals:\r\n";
    std::cout << "    2 3 4 5 6 7 8 9 T J Q K A";
    for(int x=13; x<22;x++){
        std::cout << "\r\n" << x << " ";
        if(x < 10){std::cout << " ";}
        for(int y = 2;y < 14;y++){
            hand hand;
            hand.total = x;
            hand.isSoft = 1;
            hand.canDouble = 1;
            hand.isPair = 0;
            switch(strategy.playerBasic(hand,y)){
                case decisions::HIT:
                    std::cout << " H";
                    break;
                case decisions::STAND:
                    std::cout << " S";
                    break;
                case decisions::DOUBLE:
                    std::cout << " D";
                    break;
                case decisions::SPLIT:
                    std::cout << " Q";
                    break;
                case decisions::SURRENDER:
                    std::cout << " X";
                    break;
            }
        }
        hand hand;
        hand.total = x;
        hand.isSoft = 1;
        hand.canDouble = 1;
        hand.isPair = 0;
        switch(strategy.playerBasic(hand,1)){
            case decisions::HIT:
                std::cout << " H";
                break;
            case decisions::STAND:
                std::cout << " S";
                break;
            case decisions::DOUBLE:
                std::cout << " D";
                break;
            case decisions::SPLIT:
                std::cout << " Q";
                break;
            case decisions::SURRENDER:
                std::cout << " X";
                break;
        }
    }
    std::cout << "\r\n\r\n";

    std::cout << "Pairs:\r\n";
    std::cout << "    2 3 4 5 6 7 8 9 T J Q K A";
    for(int x=0; x<10;x++){
        std::cout << "\r\n" << x + 1 << " ";
        if(x + 1 < 10){std::cout << " ";}
        for(int y = 2;y < 14;y++){
            hand hand;
            hand.discard();
            hand.canSplit = 1;
            hand.canDouble = 1;
            hand.canSurrender = 1;
            hand.addCard(x);
            hand.addCard(x);
            switch(strategy.playerBasic(hand,y)){
                case decisions::HIT:
                    std::cout << " H";
                    break;
                case decisions::STAND:
                    std::cout << " S";
                    break;
                case decisions::DOUBLE:
                    std::cout << " D";
                    break;
                case decisions::SPLIT:
                    std::cout << " Q";
                    break;
                case decisions::SURRENDER:
                    std::cout << " X";
                    break;
            }
        }
        hand hand;
        hand.addCard(x);
        hand.addCard(x);
        hand.canSplit = 1;
        hand.canSurrender = 1;
        hand.canDouble = 1;
        switch(strategy.playerBasic(hand,1)){
            case decisions::HIT:
                std::cout << " H";
                break;
            case decisions::STAND:
                std::cout << " S";
                break;
            case decisions::DOUBLE:
                std::cout << " D";
                break;
            case decisions::SPLIT:
                std::cout << " Q";
                break;
            case decisions::SURRENDER:
                std::cout << " X";
                break;
        }
    }
    
    std::cout << "\r\n";
  
    return;
}

int main() {
    test();
    return 420;
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

        int upCard = 0;        
        
        //deal 2 cards to everyone
        for(int x = 0;x<2;x++){
            dealer.addCard(shoe.back());
            if(x%2 == 1){
                upCard = card().value(shoe.back());
            }
            shoe.pop_back();
            players[0].hands[0].addCard(shoe.back());
            shoe.pop_back();
        }

        //check for dealer Ace up
        //TODO: Offer insurance
        //debug print if ace up
        if(upCard == 1){
            std::cout << "Dealer has ace!\r\n";
        }

        //check for dealer blackjack
        //TODO: take bets or push hands, clean up hands and deal next round
        if(dealer.total == 21 || (dealer.total == 11 && dealer.isSoft)){
            std::cout << "Dealer has blackjack!\r\n";
        //if the dealer didn't have blackjack, play the round...
        }else{
            
            
            //player turns
            decision = strategy.playerBasic(players[0].hands[0],upCard);
            while(decision != decisions::STAND){
                if(decision == decisions::HIT|| decision == decisions::DOUBLE || decision == decisions::SPLIT){
                    players[0].hands[0].addCard(shoe.back());
                    shoe.pop_back();
                    decision = strategy.playerBasic(players[0].hands[0],upCard);
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
                    std::cout << "Player won!\r\n";
                }
                if(players[0].hands[0].total == dealer.total){
                    std::cout << "Player push\r\n";
                }
                if(players[0].hands[0].total < dealer.total && dealer.total <= 21){
                    std::cout << "Player lost\r\n";
                }
            }else{
                std::cout << "Player BUST!\r\n";
            }
        }
        //debug print cards
        std::cout << "Dealer: ";
        dealer.print();
        std::cout << "Player: ";
        players[0].hands[0].print();
        std::cout << "Cards left: " << shoe.size() << "\r\n \r\n";
        
        
        dealer.discard();
        players[0].hands[0].discard();
        
    }   




    
    return 0;
}