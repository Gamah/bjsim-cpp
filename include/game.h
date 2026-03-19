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
    void runGame(xoshiro256pp rengine, long& shoesPlayed, std::mutex& processResults, std::vector<player>& playersPlayed);
};