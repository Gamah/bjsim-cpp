#pragma once
#include <string>
namespace card{    
    static constexpr int values[13] = {1,2,3,4,5,6,7,8,9,10,10,10,10};
    static const std::string suits[4] = {"H","D","S","C"};
    static const std::string faces[13] = {"A","2","3","4","5","6","7","8","9","T","J","Q","K"};
    static const std::string decks[8] = {"1","2","3","4","5","6","7","8"};

    int value(int cardIndex);
    std::string face(int cardIndex);
    std::string suit(int cardIndex);
    std::string deck(int cardIndex);
    std::string print(int cardIndex);
};