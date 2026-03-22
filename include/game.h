#pragma once
#include "strategies.h"
#include "utilities.h"
#include "hand.h"
#include "player.h"
#include "shoe.h"
#include "card.h"
#include "game.h"
#include "xoshiro.h"
#include <mutex>

namespace game{
    // Native multi-threaded entry point
    void runGame(xoshiro256pp rengine, long& shoesPlayed, std::mutex& processResults, std::vector<player>& playersPlayed);
    // WASM / single-threaded entry point: runs numShoes shoes, accumulates directly into players
    void runBatch(xoshiro256pp rengine, int numShoes, std::vector<player>& players);
};