#include "include/utilities.h"
#include <string>
#include <fstream>
#include <iostream>
#include "include/json.hpp"
void debugPrint(std::string string){
    if(config::settings::debug){
        std::cout << string << "\r\n";
    }
    return;
}

void config::doSetup(){
    std::ifstream cfgFile("config.json");
    nlohmann::json cfg;
    cfg << cfgFile;
    config::rules::H17 = cfg["Rules"]["H17"];
    config::rules::DAS = cfg["Rules"]["DAS"];
    config::rules::RSA = cfg["Rules"]["RSA"];
    config::rules::Surrender = cfg["Rules"]["Surrender"];
    config::rules::maxSplit = cfg["Rules"]["maxSplit"];
    config::rules::deckPen = cfg["Rules"]["deckPen"];

    config::settings::debug = cfg["Config"]["debug"];
    config::settings::numThreads = cfg["Config"]["numThreads"];
    config::settings::numShoes = cfg["Config"]["numShoes"];
}