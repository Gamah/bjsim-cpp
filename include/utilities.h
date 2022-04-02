#pragma once
#include <string>
#include <iostream>
#include "json.hpp"

namespace config{
    struct settings{
        inline static bool debug;
        inline static long numThreads;
        inline static long numShoes;
    };

    struct rules{
        inline static bool H17;
        inline static bool DAS;
        inline static bool RSA;
        inline static bool Surrender;
        inline static int maxSplit;
        inline static int deckPen;
    };
    
    void doSetup();
};

void debugPrint(std::string string);

enum class decisions{HIT, STAND, DOUBLE, SPLIT, SURRENDER};

namespace handResults{
    static const int doublelose = 0;
    static const int lose = 1;
    static const int surrender = 2;
    static const int insurancelose = 3;
    static const int insurancewin = 4;
    static const int push = 5;
    static const int win = 6;
    static const int blackjack = 7;
    static const int doublewin = 8;
    static const int roudsplayed = 9;
}