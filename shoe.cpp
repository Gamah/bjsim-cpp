#include "include/shoe.h"
#include "include/utilities.h"
#include <numeric>

void shoe::shuffleCards(xoshiro256pp& rengine){
    int n = 52 * config::rules::numDecks;
    cards.resize(n);
    std::iota(cards.begin(), cards.end(), 0);
    for(int i = n - 1; i > 0; i--){
        int j = (int)rengine.bounded_rand((uint64_t)(i + 1));
        int tmp = cards[i];
        cards[i] = cards[j];
        cards[j] = tmp;
    }
    runningCount = 0;
}
