#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <string>
#include <vector>
#include <chrono>
#include "include/config.h"
#include "include/player.h"
#include "include/game.h"
#include "include/xoshiro.h"
#include "include/utilities.h"
#include "include/json.hpp"

static std::vector<player> g_players;
static xoshiro256pp* g_rengine = nullptr;

static void applyConfig(const nlohmann::json& cfg){
    config::settings::debug    = false;
    config::rules::H17             = cfg.value("H17", true);
    config::rules::DAS             = cfg.value("DAS", true);
    config::rules::RSA             = cfg.value("RSA", true);
    config::rules::Surrender       = cfg.value("Surrender", 0);
    config::rules::BJ65            = cfg.value("BJ65", false);
    config::rules::maxSplit        = cfg.value("maxSplit", 4);
    config::rules::numDecks        = cfg.value("numDecks", 6);
    config::rules::deckPen         = cfg.value("deckPen", 52);
    config::rules::numOtherPlayers = cfg.value("numOtherPlayers", 0);
}

extern "C" {

// Configure the simulation from a JSON string. Call this before run_batch.
EMSCRIPTEN_KEEPALIVE
void bjsim_configure(const char* json_config){
    nlohmann::json cfg = nlohmann::json::parse(json_config);
    applyConfig(cfg);

    // Seed RNG
    uint64_t seed = (uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();
    delete g_rengine;
    g_rengine = new xoshiro256pp(seed);

    // Build player list: one tracked player + N dummy players to consume cards
    g_players.clear();
    g_players.emplace_back("Player", 2);
    for(int i = 0; i < config::rules::numOtherPlayers; i++){
        g_players.emplace_back("Dummy" + std::to_string(i), 0);
    }
}

// Reset accumulated results (keeps config). Call before rerunning with same config.
EMSCRIPTEN_KEEPALIVE
void bjsim_reset(){
    g_players.clear();
    g_players.emplace_back("Player", 2);
    for(int i = 0; i < config::rules::numOtherPlayers; i++){
        g_players.emplace_back("Dummy" + std::to_string(i), 0);
    }
    // Re-seed so repeated runs don't produce identical results
    uint64_t seed = (uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();
    delete g_rengine;
    g_rengine = new xoshiro256pp(seed);
}

// Run num_shoes shoes and accumulate results. Safe to call repeatedly.
// JS workers call this in a loop with a small batch size to allow progress updates.
EMSCRIPTEN_KEEPALIVE
void bjsim_run_batch(int num_shoes){
    if(!g_rengine || g_players.empty()) return;
    game::runBatch(*g_rengine, num_shoes, g_players);
}

// Return current results as a JSON string.
// Array layout: index 0 = TC -8.0, index 32 = TC 0.0, index 64 = TC +8.0 (0.25 steps).
// Only reports the tracked player (index 0); dummy players are excluded.
EMSCRIPTEN_KEEPALIVE
const char* bjsim_get_results(){
    static std::string resultBuf;

    nlohmann::json out;
    const player& p = g_players[0];

    for(int y = 0; y < 10; y++){
        nlohmann::json arr = nlohmann::json::array();
        for(int i = 0; i < 65; i++){
            arr.push_back(p.handResults[i][y]);
        }
        out[handResults::handType[y]] = arr;
    }

    resultBuf = out.dump();
    return resultBuf.c_str();
}

} // extern "C"
#endif // __EMSCRIPTEN__
