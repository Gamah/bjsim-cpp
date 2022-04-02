#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <mutex>
#include <thread>
#include <fstream>
#include "include/game.h"
#include "include/utilities.h"


int main(){
    config::doSetup();

    std::vector<std::thread> threads;
    std::mutex processResults;
    std::vector<player> playersPlayed;

    player finalPlayer;
    std::vector<long> shoesPlayed;

    for(int x = 0;x<config::settings::numThreads;x++){
        shoesPlayed.push_back(0);
    }

    for(int x=0;x<config::settings::numThreads;x++){
        std::mt19937 newRengine(time(nullptr) + x);
        threads.push_back(std::thread(game::runGame,newRengine,std::ref(shoesPlayed[x]),std::ref(processResults),std::ref(playersPlayed)));
    }
    if(config::settings::numThreads == 1 && config::settings::debug){
        for(std::thread& t : threads){
            t.join();
        }    
    }else{
        for(std::thread& t : threads){
            t.detach();
        }

        std::cout << "Sim started!" << std::endl << std::endl;
        bool threadsRunning = true;
        while(threadsRunning){

            long currentProgress = 0L;
            threadsRunning = false;
            for(long& i : shoesPlayed){

                currentProgress += i;
                if(i < config::settings::numShoes){
                    threadsRunning = true;
                }
            }
            long totalProgress = config::settings::numShoes * config::settings::numThreads;
            long percentProgress = (currentProgress*100L/totalProgress);
            std::this_thread::sleep_for (std::chrono::milliseconds(100));
            if(threadsRunning){
                std::cout << "\rWorking... [";
                for(long x = 0L;x < 50L; x++){
                    if(x < percentProgress/2L){
                        std::cout << "=";
                    }else{
                        std::cout << " ";
                    }
                }
                std::cout << "] " << percentProgress << "%" << std::flush;
            }
        }
        //i'm lazy, this works...
        std::cout << "\rWorking... [==================================================] 100%" << std::endl << std::endl;
    }
    
    //combine results from players
    for(player& p : playersPlayed){
        for(int x=0;x<15;x++){
            for(int y=0;y<10;y++){
                finalPlayer.handResults[x][y] += p.handResults[x][y];
            }
        }
    }

    std::ofstream outfile;
    outfile.open("out.csv");
    outfile << "count,doublelose,lose,surrender,insurancelose,insurancewin,push,win,blackjack,doublewin,roundsplayed";
        for(int x = -7; x <= 7;x++){
            outfile << "\r\n" << x << ",";
            for(int y = 0; y < 10; y++){
                outfile << finalPlayer.handResults[x+7][y] << ",";
            }
        }
    outfile << std::endl;
    outfile.close();

    return 0;
}