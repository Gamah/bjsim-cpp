#pragma once

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
        inline static int Surrender;  // 0=None, 1=Late, 2=Early
        inline static bool BJ65;      // true=6:5 payout, false=3:2
        inline static int maxSplit;
        inline static int numDecks;
        inline static int deckPen;
        inline static int numOtherPlayers; // 0-6 other players at table
    };
};
