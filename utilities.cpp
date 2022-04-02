#include "include/utilities.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "include/json.hpp"
void debugPrint(std::string string){
    if(config::settings::debug){
        std::cout << string << std::endl;
    }
    return;
}

void config::doSetup(){
    std::ifstream cfgFile("config.json");
    nlohmann::json cfg;
    cfgFile >> cfg;

    config::settings::debug = cfg["Config"]["debug"];
    config::settings::numThreads = cfg["Config"]["numThreads"];
    config::settings::numShoes = cfg["Config"]["numShoes"];
    config::settings::numPlayers = cfg["Players"].size();

    config::rules::H17 = cfg["Rules"]["H17"];
    config::rules::DAS = cfg["Rules"]["DAS"];
    config::rules::RSA = cfg["Rules"]["RSA"];
    config::rules::Surrender = cfg["Rules"]["Surrender"];
    config::rules::maxSplit = cfg["Rules"]["maxSplit"];
    config::rules::numDecks = cfg["Rules"]["numDecks"];
    config::rules::deckPen = cfg["Rules"]["deckPen"];
}

std::vector<player> config::getPlayers(){
    std::ifstream cfgFile("config.json");
    nlohmann::json cfg;
    cfgFile >> cfg;
    std::vector<player> players;

    for(auto it = cfg["Players"].begin(); it != cfg["Players"].end(); ++it){
        player newPlayer(it.value()["Name"]);
        players.push_back(newPlayer);
    }

    return players;
}