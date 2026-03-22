#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <thread>
#include "include/game.h"
#include "include/xoshiro.h"
#include "include/utilities.h"
#include "include/json.hpp"


int main(){
    config::doSetup();

    std::vector<std::thread> threads;
    std::mutex processResults;
    std::vector<player> playersPlayed = config::getPlayers();
    std::vector<long> shoesPlayed(config::settings::numThreads, 0);

    for(int x = 0; x < config::settings::numThreads; x++){
        uint64_t seed = (uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count() + x;
        xoshiro256pp newRengine(seed);
        threads.push_back(std::thread(game::runGame, newRengine, std::ref(shoesPlayed[x]), std::ref(processResults), std::ref(playersPlayed)));
    }

    if(config::settings::numThreads == 1 && config::settings::debug){
        for(std::thread& t : threads) t.join();
    }else{
        for(std::thread& t : threads) t.detach();

        std::cerr << "Sim started!" << std::endl << std::endl;
        bool threadsRunning = true;
        while(threadsRunning){
            long currentProgress = 0L;
            threadsRunning = false;
            for(long& i : shoesPlayed){
                currentProgress += i;
                if(i < config::settings::numShoes) threadsRunning = true;
            }
            long totalProgress = config::settings::numShoes * config::settings::numThreads;
            long percentProgress = (currentProgress * 100L / totalProgress);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if(threadsRunning){
                std::cerr << "\rWorking... [";
                for(long x = 0L; x < 50L; x++){
                    std::cerr << (x < percentProgress / 2L ? "=" : " ");
                }
                std::cerr << "] " << percentProgress << "%" << std::flush;
            }
        }
        std::cerr << "\rWorking... [==================================================] 100%" << std::endl << std::endl;
    }

    // Output results for the real player only (index 0)
    player& p = playersPlayed[0];
    nlohmann::json outjson;
    for(int i = 0; i < 65; i++){
        float tc = (i - 32) / 4.0f;
        std::string tcKey = std::to_string(tc);
        for(int y = 0; y < 10; y++){
            outjson[handResults::handType[y]][tcKey] = p.handResults[i][y];
        }
    }

    std::cout << outjson.dump(2) << std::endl;
    return 0;
}

