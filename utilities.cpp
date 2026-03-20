#include "include/utilities.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "include/json.hpp"

void config::doSetup(){
    std::ifstream cfgFile("config.json");
    nlohmann::json cfg;
    cfgFile >> cfg;

    config::settings::debug    = cfg["Config"]["debug"];
    config::settings::numThreads = cfg["Config"]["numThreads"];
    config::settings::numShoes   = cfg["Config"]["numShoes"];

    config::rules::H17             = cfg["Rules"]["H17"];
    config::rules::DAS             = cfg["Rules"]["DAS"];
    config::rules::RSA             = cfg["Rules"]["RSA"];
    config::rules::Surrender       = cfg["Rules"]["Surrender"];
    config::rules::BJ65            = cfg["Rules"]["BJ65"];
    config::rules::maxSplit        = cfg["Rules"]["maxSplit"];
    config::rules::numDecks        = cfg["Rules"]["numDecks"];
    config::rules::deckPen         = cfg["Rules"]["deckPen"];
    config::rules::numOtherPlayers = cfg["Rules"]["numOtherPlayers"];
    if(cfg["Rules"].contains("playerStrategy")){
        config::rules::playerStrategy = cfg["Rules"]["playerStrategy"];
    }
    if(cfg["Rules"].contains("deviationMask")){
        config::rules::deviationMask = cfg["Rules"]["deviationMask"];
    }
}

// Returns one real player plus numOtherPlayers dummy players (strategy 0).
// Dummy players exist only to consume cards from the shoe, matching real table conditions.
std::vector<player> config::getPlayers(){
    std::vector<player> players;
    players.emplace_back("Player", config::rules::playerStrategy);
    for(int i = 0; i < config::rules::numOtherPlayers; i++){
        players.emplace_back("Dummy" + std::to_string(i), 0);
    }
    return players;
}
