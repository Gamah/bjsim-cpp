#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include "utilities.h"
#include "strategies.h"


// random generator function:
int randomizer (int i) { return std::rand()%i;};

int main() {
    
    //initialize shoe
    //TODO: break this out of main so it can be threaded
    std::srand ( unsigned ( std::time(0) ) );
    std::vector<int> shoe;

    hand dealer;    
    hand player;
    
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
            player.addCard(shoe.back());
            shoe.pop_back();
        }

        //check for dealer Ace up
        //TODO: Offer insurance
        //debug print if ace up
        if(upCard == 1){
            std::cout << "Dealer has ace!\r\n";
        }

        //debug print cards
        std::cout << "Dealer: ";
        for(int x = 0; x < dealer.cards.size(); x++){
            std::cout << card().print(dealer.cards[x]);
        }
        std::cout << " Total: " << dealer.total << " Soft: " << dealer.isSoft << " Pair: " << dealer.isPair << "\r\n";

        std::cout << "Player: ";
        for(int x = 0; x < player.cards.size(); x++){
            std::cout << card().print(player.cards[x]);
        }
        std::cout << " Total: " << player.total << " Soft: " << player.isSoft << " Pair: " << player.isPair << "\r\n";

        std::cout << "Cards left: " << shoe.size() << "\r\n \r\n";

        dealer.discard();
        player.discard();
        
    }   




    
    return 0;
}