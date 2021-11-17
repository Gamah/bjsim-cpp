#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <random>
#include <algorithm>
#include "../include/utilities.h"
#include "../include/strategies.h"

void BStest(){
    strategies strategy;
    std::cout << "Hard Totals:\r\n";
    std::cout << "    2 3 4 5 6 7 8 9 T J Q K A";
    for(int x=5; x<22;x++){
        std::cout << "\r\n" << x << " ";
        if(x < 10){std::cout << " ";}
        for(int y = 2;y < 14;y++){
            hand hand;
            hand.total = x;
            hand.isSoft = false;
            hand.isPair = 0;
            hand.canDouble = true;
            hand.canSplit = 1;
            hand.canSurrender = true;
            switch(strategy.playerBasic(hand.total,card::value(y),hand.isPair,hand.isSoft,hand.canSplit,hand.canDouble,hand.canSurrender)){
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
            hand.discard();
        }
        hand hand;
        hand.total = x;
        hand.isSoft = false;
        hand.isPair = 0;
        hand.canDouble = true;
        hand.canSurrender = true;
        switch(strategy.playerBasic(hand.total,1,hand.isPair,hand.isSoft,hand.canSplit,hand.canDouble,hand.canSurrender)){
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
        hand.discard();
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
            hand.isSoft = true;
            hand.canDouble = true;
            hand.isPair = 0;
            switch(strategy.playerBasic(hand.total,card::value(y),hand.isPair,hand.isSoft,hand.canSplit,hand.canDouble,hand.canSurrender)){
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
            hand.discard();
        }
        hand hand;
        hand.total = x;
        hand.isSoft = true;
        hand.canDouble = true;
        hand.isPair = 0;
        switch(strategy.playerBasic(hand.total,1,hand.isPair,hand.isSoft,hand.canSplit,hand.canDouble,hand.canSurrender)){
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
        hand.discard();
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
            hand.canDouble = true;
            hand.canSurrender = true;
            hand.addCard(x);
            hand.addCard(x);
            switch(strategy.playerBasic(hand.total,card::value(y),hand.isPair,hand.isSoft,hand.canSplit,hand.canDouble,hand.canSurrender)){
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
            hand.discard();
        }
        hand hand;
        hand.addCard(x);
        hand.addCard(x);
        hand.canSplit = 1;
        hand.canSurrender = true;
        hand.canDouble = true;
        switch(strategy.playerBasic(hand.total,1,hand.isPair,hand.isSoft,hand.canSplit,hand.canDouble,hand.canSurrender)){
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
        hand.discard();
    }
    
    std::cout << "\r\n";
  
    return;
}

void soft17test(){
    hand hand;
    strategies strat;
    hand.discard();
    hand.addCard(5);
    hand.addCard(0);
    hand.print();
    switch(strat.dealerH17(hand)){
        case decisions::STAND: 
            std::cout << "STAND\r\n";
            break;
        case decisions::HIT:
            std::cout << "HIT\r\n";
            break;
    };
    hand.addCard(0);
    hand.print();
    switch(strat.dealerH17(hand)){
        case decisions::STAND: 
            std::cout << "STAND\r\n";
            break;
        case decisions::HIT:
            std::cout << "HIT\r\n";
            break;
    };
    hand.addCard(0);
    hand.print();
    switch(strat.dealerH17(hand)){
        case decisions::STAND: 
            std::cout << "STAND\r\n";
            break;
        case decisions::HIT:
            std::cout << "HIT\r\n";
            break;
    };
    hand.addCard(0);
    hand.print();
    switch(strat.dealerH17(hand)){
        case decisions::STAND: 
            std::cout << "STAND\r\n";
            break;
        case decisions::HIT:
            std::cout << "HIT\r\n";
            break;
    };
    hand.addCard(0);
    hand.print();
    switch(strat.dealerH17(hand)){
        case decisions::STAND: 
            std::cout << "STAND\r\n";
            break;
        case decisions::HIT:
            std::cout << "HIT\r\n";
            break;
    };
    hand.addCard(0);
    hand.print();
    switch(strat.dealerH17(hand)){
        case decisions::STAND: 
            std::cout << "STAND\r\n";
            break;
        case decisions::HIT:
            std::cout << "HIT\r\n";
            break;
    };

}
void aceTest(){
    hand hand;
    hand.discard();
    hand.addCard(0);
    hand.print();
    for(int x = 0; x < 22;x++){
        hand.addCard(0);
        hand.print();
    }
}

void splitTest(){
    strategies strat;
    std::vector<player> players;
    players.push_back(player());
    for(player& p : players){
        hand firstHand;
        firstHand.addCard(2);
        firstHand.addCard(2);
        p.addHand(firstHand);
        decisions decision;

        decision = strat.playerBasic(firstHand.total,5,firstHand.isPair,firstHand.isSoft,firstHand.canSplit,firstHand.canDouble,firstHand.canSurrender);
        while(decision != decisions::STAND){
            switch(decision){
                case decisions::STAND: 
                    std::cout << "STAND\r\n";
                    break;
                case decisions::HIT:
                    std::cout << "HIT\r\n";
                    break;
                case decisions::SPLIT :{
                    hand newHand;
                    //pull card off the top if debugging is on
                    if(config::debug){
                        firstHand.cards.pop_back();
                    }                
                    //halve the hand total
                    firstHand.total == firstHand.total / 2;
                    //put the top card into the new hand            
                    newHand.addCard(firstHand.topCard);
                    //deal to the current hand
                    firstHand.addCard(2);
                    //deal to the new hand
                    newHand.addCard(2);
                    //if the hands are aces or player has 4 hands (lengh of hands + new hand not yet added) then they can't resplit
                    if(p.hands.size() + 1 == rules::maxSplit){
                        firstHand.canSplit = 0;
                        newHand.canSplit = 0;
                    }
                    //this is a dumb hack to indicate split aces because i can't read the future to prevent playing or resplitting a split ace on a future hand
                    if(card::value(firstHand.topCard) == 1){
                        firstHand.canSplit = -1;
                        newHand.canSplit = -1;
                        decision = decisions::STAND;
                    }else{
                        decision = strat.playerBasic(firstHand.total,5,firstHand.isPair,firstHand.isSoft,firstHand.canSplit,firstHand.canDouble,firstHand.canSurrender);
                    }
                    //mark hands slpit and add to player's hands
                    firstHand.isSplit = true;
                    newHand.isSplit = true;
                    p.addHand(newHand);
                    break;
                }
                case decisions::SURRENDER:
                    std::cout << "SURRENDER\r\n";
                    break;
            };
        }
    }
    for(player& p : players){
        p.print();
    }
}   

void testCounts(){
    shoe newShoe;
    newShoe.shuffleCards();
    while(newShoe.cards.size() > 25){
        int x = newShoe.getCard();
        std::cout << "Card: " << card::print(x) << " RunningCount: " << newShoe.runningCount << " TrueCount: " << newShoe.trueCount << " Cards left: " << newShoe.cards.size() << "\r\n";
    }
}

void testBJ(){
    hand hand;
    hand.discard();
    hand.addCard(0);
    hand.addCard(9);
    hand.print();
    hand.discard();
    hand.addCard(9);
    hand.addCard(0);
    hand.print();
}
int main(){
    BStest();
    return 69;
}