#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "json.hpp"
#include "config.h"
#include "player.h"

namespace config{
    void doSetup();
    std::vector<player> getPlayers();
};

inline void debugPrint(const std::string& string){
    if(config::settings::debug){
        std::cout << string << std::endl;
    }
}

enum class decisions{HIT, STAND, DOUBLE, SPLIT, SURRENDER};

namespace dev {
    constexpr uint32_t SPLIT_10_VS_4 = 1u << 0;   // Split 10s vs 4, TC≥6
    constexpr uint32_t SPLIT_10_VS_5 = 1u << 1;   // Split 10s vs 5, TC≥5
    constexpr uint32_t SPLIT_10_VS_6 = 1u << 2;   // Split 10s vs 6, TC≥4
    constexpr uint32_t SOFT19_D4     = 1u << 3;   // Soft 19 double vs 4, TC≥3
    constexpr uint32_t SOFT19_D5     = 1u << 4;   // Soft 19 double vs 5, TC≥1
    constexpr uint32_t SOFT19_S6     = 1u << 5;   // Soft 19 stand vs 6, RC<0
    constexpr uint32_t SOFT17_D2     = 1u << 6;   // Soft 17 double vs 2, TC≥1
    constexpr uint32_t H16_S9        = 1u << 7;   // Hard 16 stand vs 9, TC≥4
    constexpr uint32_t H16_S10       = 1u << 8;   // Hard 16 stand vs 10, RC≥0
    constexpr uint32_t H16_SA        = 1u << 9;   // Hard 16 stand vs A, TC≥3
    constexpr uint32_t H15_S10       = 1u << 10;  // Hard 15 stand vs 10, TC≥4
    constexpr uint32_t H15_SA        = 1u << 11;  // Hard 15 stand vs A, TC≥5
    constexpr uint32_t H13_H2        = 1u << 12;  // Hard 13 hit vs 2, TC≤-1
    constexpr uint32_t H12_S2        = 1u << 13;  // Hard 12 stand vs 2, TC≥3
    constexpr uint32_t H12_S3        = 1u << 14;  // Hard 12 stand vs 3, TC≥2
    constexpr uint32_t H12_H4        = 1u << 15;  // Hard 12 hit vs 4, RC<0
    constexpr uint32_t H10_D10       = 1u << 16;  // Hard 10 double vs 10, TC≥4
    constexpr uint32_t H10_DA        = 1u << 17;  // Hard 10 double vs A, TC≥3
    constexpr uint32_t H9_D2         = 1u << 18;  // Hard 9 double vs 2, TC≥1
    constexpr uint32_t H9_D7         = 1u << 19;  // Hard 9 double vs 7, TC≥3
    constexpr uint32_t H8_D6         = 1u << 20;  // Hard 8 double vs 6, TC≥2
    constexpr uint32_t H17_SUR_A     = 1u << 21;  // Hard 17 surrender vs A (H17)
    constexpr uint32_t H16_SUR_8     = 1u << 22;  // Hard 16 surrender vs 8, TC≥4
    constexpr uint32_t H16_H9        = 1u << 23;  // Hard 16 hit vs 9, TC≤-1
    constexpr uint32_t H16_SUR_TA    = 1u << 24;  // Hard 16 surrender vs 10/A (H17 games)
    constexpr uint32_t H15_SUR_9     = 1u << 25;  // Hard 15 surrender vs 9, TC≥2
    constexpr uint32_t H15_H10       = 1u << 26;  // Hard 15 hit vs 10, RC<0
    constexpr uint32_t H15_SUR_A     = 1u << 27;  // Hard 15 surrender vs A, TC≤-1
    constexpr uint32_t ALL           = 0x0FFFFFFFu;
}

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

    inline std::string handType[] = {
        "doublelose",
        "lose",
        "surrender",
        "insurancelose",
        "insurancewin",
        "push",
        "win",
        "blackjack",
        "doublewin",
        "roudsplayed"
    };
    
}