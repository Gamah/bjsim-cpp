#pragma once
#include <string>
namespace config{
    static const bool debug = true;
    static const int numThreads = 1;
    static const int numShoes = 3;
};
void debugPrint(std::string string);

enum class decisions{HIT, STAND, DOUBLE, SPLIT, SURRENDER};
namespace rules{
    static const bool H17 = true;
    static const bool DAS = true;
    static const bool RSA = false;
    static const bool Surrender = false;
    static const int maxSplit = 4;
    static const int deckPen = 78;
};
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