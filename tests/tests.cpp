#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <random>
#include <algorithm>
#include "../include/utilities.h"
#include "../include/strategies.h"
#include <list>

void BStest(){
    strategies strategy;
    std::cout << "Hard Totals:\r\n";
    std::cout << "    2 3 4 5 6 7 8 9 T J Q K A";
    for(int x=5; x<22;x++){
        std::cout << "\r\n" << x << " ";
        if(x < 10){std::cout << " ";}
        for(int y = 1;y < 13;y++){
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
        for(int y = 1;y < 13;y++){
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
        for(int y = 1;y < 13;y++){
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

        hand h;
        h.addCard(2);
        h.addCard(2);
        p.addHand(h);
        decisions decision;
        for(hand& h : p.hands){
            int thishand=1;
            decision = strat.playerBasic(h.total,5,h.isPair,h.isSoft,h.canSplit,h.canDouble,h.canSurrender);
            while(decision != decisions::STAND){
                if(h.canSplit == -1){
                    decision = decisions::STAND;
                }else{
                    switch(decision){
                        case decisions::STAND: 
                            std::cout << "STAND\r\n";
                            break;
                        case decisions::HIT:
                            std::cout << "HIT\r\n";
                            decision = decisions::STAND;
                            break;
                        case decisions::DOUBLE:
                            decision = decisions::STAND;
                            break;
                        case decisions::SPLIT : {
                            //new hand object for after dealing with this hand...
                            hand newhand;
                            //pull card off the top if debugging is on
                            if(config::debug){
                                h.cards.pop_back();
                            }                    
                            //halve the hand total
                            h.total == h.total / 2;
                            //put the top card into the new hand            
                            newhand.addCard(h.topCard);
                            //deal to the current hand
                            h.addCard(2);
                            //deal to the new hand
                            newhand.addCard(1);
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
                                std::cout << thishand;
                                h.print();
                                decision = strat.playerBasic(h.total,5,h.isPair,h.isSoft,h.canSplit,h.canDouble,h.canSurrender);
                            }
                            //mark hands slpit and add to player's hands
                            h.isSplit = true;
                            newhand.isSplit = true;
                            p.addHand(newhand);
                            break;
                        }
                        case decisions::SURRENDER:{   
                            std::cout << "SURRENDER\r\n";
                            break;
                        }
                    }
                }
            }
            thishand++;
        }
    for(player& p : players){
        p.print();
    }
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

void bjTest(){
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

void listTest(){
    std::list<int> mylist;
    mylist.push_back(1);
    mylist.push_back(2);
    mylist.push_back(3);
    mylist.push_back(4);
    mylist.push_back(5);

    for(int& i : mylist){
        if(i == 5){
            mylist.push_back(9);
        }
        std::cout << i;
    }
}

int main(){
    BStest();
    return 69;
}