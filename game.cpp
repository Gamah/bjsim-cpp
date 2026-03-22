#include "include/xoshiro.h"
#include "include/strategies.h"
#include "include/utilities.h"
#include "include/hand.h"
#include "include/player.h"
#include "include/shoe.h"
#include "include/card.h"
#include "include/game.h"
#include <mutex>

// Core simulation loop. Runs numShoes shoes and accumulates results directly
// into the provided players vector. shoesCompleted is optional (native progress tracking).
static void runShoes(xoshiro256pp& rengine, int numShoes, std::vector<player>& players, long* shoesCompleted = nullptr) {
    shoe shoe;
    strategies strategy;
    decisions decision;
    hand dealer;

    for(int x = 0; x < numShoes; x++){
        shoe.shuffleCards(rengine);
        debugPrint("Shuffle!");

        if(config::settings::debug){
            for(int& c : shoe.cards){
                std::cout << card::print(c) << ",";
            }
            std::cout << "\r\n";
        }
        shoe.getCard(); // burn a card

        while(shoe.cards.size() >= (size_t)config::rules::deckPen){
            float currentTC = shoe.trueCount();

            for(player& p : players){
                hand newHand;
                newHand.discard();
                newHand.trueCount = currentTC;
                p.addResult(newHand.trueCount, handResults::roudsplayed);
                p.addHand(newHand);
            }

            int upCard = 0;
            if(config::settings::debug){
                debugPrint("RunningCount: " + std::to_string(shoe.runningCount));
                debugPrint("TrueCount: " + std::to_string(shoe.trueCount()));
                debugPrint("Cards Left: " + std::to_string(shoe.cards.size()) + "\r\n");
            }

            // Deal 2 cards to everyone
            for(int d = 0; d < 2; d++){
                for(player& p : players){
                    for(hand& h : p.hands){
                        h.addCard(shoe.getCard());
                    }
                }
                if(d % 2 == 1){
                    int newCard = shoe.getCard();
                    dealer.addCard(newCard);
                    upCard = card::value(newCard);
                }else{
                    dealer.addCard(shoe.getDownCard());
                }
            }

            // Insurance: if dealer shows Ace and TC >= 3
            if(upCard == 1){
                if(shoe.trueCount() >= 3){
                    for(player& p : players){
                        for(hand& h : p.hands){
                            h.isInsured = true;
                        }
                    }
                }
            }

            // Early surrender phase: before dealer peeks for blackjack
            if(config::rules::Surrender == 2){
                for(player& p : players){
                    for(hand& h : p.hands){
                        if(strategy.play(h, upCard, shoe, p.strategy) == decisions::SURRENDER){
                            h.isSurrendered = true;
                        }
                        // No late surrender in early surrender games
                        h.canSurrender = false;
                    }
                }
            }

            // Play the round if dealer doesn't have blackjack
            if(dealer.total != 21){
                bool dealerPlays = false;
                // Player turns
                for(player& p : players){
                    for(hand& h : p.hands){
                        if(h.isSurrendered) continue; // already early surrendered

                        decision = strategy.play(h, upCard, shoe, p.strategy);
                        while(decision != decisions::STAND){
                            if(h.canSplit == -1){
                                // Split aces: only resplit if RSA and max not hit
                                if(card::value(h.topCard) == 1 && config::rules::RSA && p.hands.size() + 1 <= (size_t)config::rules::maxSplit){
                                    h.canSplit = 1;
                                    decision = decisions::SPLIT;
                                }else{
                                    decision = decisions::STAND;
                                }
                            }else{
                                switch(decision){
                                    case decisions::HIT : {
                                        h.addCard(shoe.getCard());
                                        decision = strategy.play(h, upCard, shoe, p.strategy);
                                        break;
                                    }
                                    case decisions::SPLIT : {
                                        hand newhand;
                                        newhand.discard();
                                        int topCard = h.topCard;
                                        h.numCards--;
                                        h.isPair = 0;
                                        h.isSplit = true;
                                        newhand.isSplit = true;
                                        h.canSurrender = false;
                                        newhand.canSurrender = false;
                                        newhand.trueCount = h.trueCount;

                                        h.total = h.total - card::value(topCard);
                                        newhand.addCard(topCard);
                                        h.addCard(shoe.getCard());
                                        newhand.addCard(shoe.getCard());

                                        if(p.hands.size() + 1 == (size_t)config::rules::maxSplit){
                                            h.canSplit = 0;
                                            newhand.canSplit = 0;
                                        }
                                        if(card::value(topCard) == 1){
                                            h.canSplit = -1;
                                            newhand.canSplit = -1;
                                            if(card::value(h.topCard) == 1 && config::rules::RSA && p.hands.size() + 1 <= (size_t)config::rules::maxSplit){
                                                decision = decisions::SPLIT;
                                            }else{
                                                decision = decisions::STAND;
                                            }
                                        }else{
                                            decision = strategy.play(h, upCard, shoe, p.strategy);
                                        }
                                        p.addHand(newhand);
                                        break;
                                    }
                                    case decisions::DOUBLE : {
                                        h.addCard(shoe.getCard());
                                        h.isDoubled = true;
                                        decision = decisions::STAND;
                                        break;
                                    }
                                    case decisions::SURRENDER : {
                                        h.isSurrendered = true;
                                        decision = decisions::STAND;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                // Dealer turn: only plays if any player has a live hand <= 21
                for(player& p : players){
                    if(dealerPlays) break;
                    for(hand& h : p.hands){
                        if(h.total <= 21 && !h.isSurrendered){
                            dealerPlays = true;
                            break;
                        }
                    }
                }
                shoe.flipDownCard();
                if(dealerPlays){
                    decision = strategy.dealer(dealer);
                    while(decision != decisions::STAND){
                        if(decision == decisions::HIT){
                            dealer.addCard(shoe.getCard());
                            decision = strategy.dealer(dealer);
                        }
                    }
                }
            }else{
                shoe.flipDownCard();
            }

            // Resolve outcomes
            debugPrint("Dealer");
            if(config::settings::debug) dealer.print();

            for(player& p : players){
                for(hand& h : p.hands){
                    debugPrint("Player");
                    if(config::settings::debug) h.print();

                    if(h.isInsured){
                        if(dealer.total == 21 && dealer.numCards == 2){
                            p.addResult(h.trueCount, handResults::insurancewin);
                            debugPrint("Insurance Win");
                            continue;
                        }else{
                            p.addResult(h.trueCount, handResults::insurancelose);
                            debugPrint("Insurance Lose");
                        }
                    }

                    if(dealer.total == 21 && dealer.numCards == 2){
                        if(h.total == 21 && h.numCards == 2){
                            p.addResult(h.trueCount, handResults::push);
                            debugPrint("Blackjack Push");
                        }else{
                            p.addResult(h.trueCount, handResults::lose);
                            debugPrint("Blackjack Lose");
                        }
                        continue;
                    }

                    if(h.total == 21 && h.numCards == 2 && h.isSplit == false){
                        p.addResult(h.trueCount, handResults::blackjack);
                        debugPrint("Blackjack Win");
                        continue;
                    }

                    if(h.isSurrendered){
                        p.addResult(h.trueCount, handResults::surrender);
                        debugPrint("Surrender");
                        continue;
                    }

                    if(h.total > 21){
                        if(h.isDoubled){
                            p.addResult(h.trueCount, handResults::doublelose);
                            debugPrint("Player double bust");
                        }else{
                            p.addResult(h.trueCount, handResults::lose);
                            debugPrint("Player bust");
                        }
                        continue;
                    }

                    if(dealer.total > 21){
                        if(h.isDoubled){
                            p.addResult(h.trueCount, handResults::doublewin);
                            debugPrint("Dealer bust double win");
                        }else{
                            p.addResult(h.trueCount, handResults::win);
                            debugPrint("Dealer bust win");
                        }
                        continue;
                    }

                    if(h.total > dealer.total){
                        if(h.isDoubled){
                            p.addResult(h.trueCount, handResults::doublewin);
                            debugPrint("Player double win");
                        }else{
                            p.addResult(h.trueCount, handResults::win);
                            debugPrint("Player win");
                        }
                    }else if(h.total < dealer.total){
                        if(h.isDoubled){
                            p.addResult(h.trueCount, handResults::doublelose);
                            debugPrint("Player double lose");
                        }else{
                            p.addResult(h.trueCount, handResults::lose);
                            debugPrint("Player lose");
                        }
                    }else{
                        p.addResult(h.trueCount, handResults::push);
                        debugPrint("Player push");
                    }
                }
            }

            dealer.discard();
            for(player& p : players){
                p.clearHands();
            }
            if(config::settings::debug) std::cout << "\r\n\r\n";
        }

        if(shoesCompleted) *shoesCompleted = x + 1;
    }
}

void game::runBatch(xoshiro256pp rengine, int numShoes, std::vector<player>& players){
    runShoes(rengine, numShoes, players);
}

void game::runGame(xoshiro256pp rengine, long& shoesPlayed, std::mutex& processResults, std::vector<player>& playersPlayed){
    std::vector<player> localPlayers = playersPlayed;
    runShoes(rengine, config::settings::numShoes, localPlayers, &shoesPlayed);

    processResults.lock();
    for(int p = 0; p < (int)localPlayers.size(); p++){
        for(int c = 0; c < 65; c++){
            for(int r = 0; r < 10; r++){
                playersPlayed[p].handResults[c][r] += localPlayers[p].handResults[c][r];
            }
        }
    }
    processResults.unlock();
}
