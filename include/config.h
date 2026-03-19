#pragma once

namespace config{
    struct settings{
        inline static bool debug;
        inline static long numThreads;
        inline static long numShoes;
        inline static int numPlayers;
    };

    struct rules{
        inline static bool H17;
        inline static bool DAS;
        inline static bool RSA;
        inline static bool Surrender;
        inline static int maxSplit;
        inline static int numDecks;
        inline static int deckPen;
    };
};
