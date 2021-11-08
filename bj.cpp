#include <iostream>
#include <string>
#include <list>
#include "utilities.h"
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>


// random generator function:
int randomizer (int i) { return std::rand()%i;}

int main() {
    
    //initialize shoe
    //TODO: break this out of main so it can be threaded
    std::srand ( unsigned ( std::time(0) ) );
    std::vector<int> shoe;
    gameObjects::card card;

    gameObjects::hand dealer;    
    gameObjects::hand player;
    
    for(int x = 0; x < (52*6); x++){
        shoe.push_back(x);
    }

    // using built-in random generator:
    std::random_shuffle( shoe.begin(), shoe.end());
    
    //gameObjects::hand testhand;
    //testhand.addCard(6);
    //testhand.addCard(8);
    //for(int x = 0;x<22;x++){
    //    testhand.addCard(0);
    //    std::cout << testhand.total << "\r\n";
    //}
    //return 69;
    while(shoe.size() > 76){
        
        /*
        std::cout << card.print(shoe.back());
        shoe.pop_back();
        */
        
        for(int x = 0;x<2;x++){
            dealer.addCard(shoe.back());
            shoe.pop_back();
            player.addCard(shoe.back());
            shoe.pop_back();
        }

        std::cout << "Dealer: ";
        for(int x = 0; x < dealer.cards.size(); x++){
            std::cout << card.print(dealer.cards[x]);
        }
        std::cout << " Total: " << dealer.total << " Soft: " << dealer.isSoft << " Pair: " << dealer.isPair << "\r\n";

        std::cout << "Player: ";
        for(int x = 0; x < player.cards.size(); x++){
            std::cout << card.print(player.cards[x]);
        }
        std::cout << " Total: " << player.total << " Soft: " << player.isSoft << " Pair: " << player.isPair << "\r\n";

        std::cout << "Cards left: " << shoe.size() << "\r\n \r\n";

        dealer.discard();
        player.discard();
        
    }   




    
    return 0;
}