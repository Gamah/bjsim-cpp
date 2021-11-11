#include <iostream>
#include <string>
#include <vector>
#include "../include/utilities.h"
#include "../include/strategies.h"

void BStest(){
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
            hand.discard();
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
            hand.discard();
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
            hand.discard();
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
        hand.discard();
    }
    
    std::cout << "\r\n";
  
    return;
}
 int main(){
     BStest();
     return 69;
 }