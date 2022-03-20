#pragma once
#include "strategies.h"
#include "utilities.h"
#include "hand.h"
#include "player.h"
#include "shoe.h"
#include "card.h"
#include "game.h"
#include <mutex>

namespace game{
    void runGame(std::mt19937 rengine, long& shoesPlayed, std::mutex& processResults, std::vector<player>& playersPlayed);
};