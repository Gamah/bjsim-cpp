# bjsim-cpp

A multi-threaded blackjack Monte Carlo simulator compiled to WebAssembly for
in-browser execution. Simulates millions of shoes and buckets hand outcomes by
true count (quarter-deck precision), enabling EV and Risk of Ruin analysis with
arbitrary bet spreads.

## Quick start

On a fresh Ubuntu 24.04 server:

```bash
git clone <repo-url> bjsim-cpp
cd bjsim-cpp
sudo bash deploy.sh
```

The script installs all dependencies, builds the WASM module, runs the test
suite, and serves the app from nginx on port 80. First run takes 10–20 minutes
because Emscripten is downloaded and compiled. Subsequent runs are fast.

## Web app

Open `http://<server-ip>/` after deploying.

**Table Rules**

| Setting | Description |
|---|---|
| Decks | 1, 2, 4, 6, or 8 |
| Deck penetration | Decks behind cut card (0–half the shoe, in ¼-deck steps) |
| H17 | Dealer hits soft 17 |
| Blackjack pays 6:5 | Toggle: 6:5 (1.2×) vs standard 3:2 (1.5×) |
| DAS | Double after split |
| RSA | Resplit aces |
| Surrender | None / Late / Early |
| Max splits | 2, 3, or 4 |
| Other players | 0–6 additional players at the table (affects shoe penetration) |

**Simulation**

| Setting | Description |
|---|---|
| Shoes | 10K, 50K, 100K, 500K, or 1M |
| CPU threads | One Web Worker + WASM instance per thread; default is all cores minus one |

**Betting strategy**

Each row sets a true count threshold, number of simultaneous hands (1–3), and
bet size in dollars. The first row (no threshold) is the flat bet used below all
thresholds. Thresholds are selectable in 0.25 true-count increments from −4 to
+8.

**Session analysis**

Rounds per hour and optional starting bankroll. With a bankroll set, the app
calculates Risk of Ruin using the standard Gambler's Ruin formula.

**Results** update in real time as each worker batch completes:

- EV per hour ($) at your bet spread and pace
- House edge (%, unweighted flat bet)
- Risk of Ruin (%)
- EV by true count chart
- Round frequency by true count chart
- Full outcome breakdown table (collapsible)

**Export** — the Export JSON button downloads a result blob keyed by config
hash, structured for easy server-side caching.

---

## Building manually

Requires a C++17 compiler and `make`.

```bash
# Native CLI binary (outputs JSON to stdout)
make native

# WebAssembly module (outputs public/bjsim.js + public/bjsim.wasm)
make wasm        # requires emcc on PATH

# Test suite
make test && ./tests/tests
```

Manual build commands without make:

```bash
# Native
g++ -O3 -std=c++17 -pthread -o bjsim \
    game.cpp player.cpp strategies.cpp shoe.cpp hand.cpp card.cpp utilities.cpp bjsim.cpp

# WASM
emcc -O3 -std=c++17 \
    -s EXPORTED_FUNCTIONS='["_bjsim_configure","_bjsim_run_batch","_bjsim_reset","_bjsim_get_results"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s MODULARIZE=1 -s EXPORT_NAME=BJSim \
    -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=worker \
    -o public/bjsim.js \
    game.cpp player.cpp strategies.cpp shoe.cpp hand.cpp card.cpp utilities.cpp wasm_api.cpp

# Tests
g++ -O2 -std=c++17 -o tests/tests \
    tests/tests.cpp hand.cpp shoe.cpp strategies.cpp utilities.cpp card.cpp player.cpp
```

Profiling build:

```bash
g++ -O3 -pg -std=c++17 -pthread -o bjsim_prof \
    game.cpp player.cpp strategies.cpp shoe.cpp hand.cpp card.cpp utilities.cpp bjsim.cpp
```

---

## Architecture

**WASM threading model** — the browser spawns N Web Workers (one per CPU thread
selected), each loading its own independent WASM instance. Workers run a slice
of the total shoes and `postMessage` partial results back to the main thread
after every 500-shoe batch. The main thread merges worker arrays element-wise
and recomputes all statistics on each update. No `SharedArrayBuffer` is
required, so no special COOP/COEP headers are needed.

**True count bucketing** — the shoe tracks count at quarter-deck precision
(`float trueCount()`). Results are bucketed into 65 slots covering TC −8.0 to
+8.0 in 0.25 steps. Index formula: `floor(tc × 4) + 32`, clamped to [0, 64].

**Strategy** — the single tracked player always uses strategy 2 (basic strategy
+ Illustrious 18 Hi-Lo deviations). Other-players-at-table seats use dealer
strategy (stand on 17+) and exist only to consume cards from the shoe.

**EV calculation** is done in JavaScript after the simulation:

```
EV/round = Σ_tc [ freq(tc) × ev_per_unit(tc) × bet(tc) × hands(tc) ]
EV/hour  = EV/round × rounds_per_hour
```

Payoffs per outcome type:

| Outcome | Units |
|---|---|
| win | +1 |
| blackjack | +1.5 (or +1.2 for 6:5) |
| doublewin | +2 |
| push, insurancewin | 0 |
| lose | −1 |
| doublelose | −2 |
| surrender | −0.5 |
| insurancelose | −0.5 (side bet; main bet resolved separately) |

**Risk of Ruin** uses the infinite-play Gambler's Ruin formula:

```
RoR = e^(−2 × bankroll × EV_per_round / Var_per_round)
```

where variance is derived from the squared payoffs across all outcome buckets.

---

## CLI configuration (`config.json`)

The native `bjsim` binary reads `config.json` from the working directory.

```jsonc
{
  "Config": {
    "numThreads": 4,       // parallel simulation threads
    "numShoes":   100000,  // shoes per thread
    "debug":      false    // per-hand trace (single-thread only)
  },
  "Rules": {
    "H17":             true,  // dealer hits soft 17
    "DAS":             true,  // double after split
    "RSA":             true,  // resplit aces
    "Surrender":       1,     // 0 = none, 1 = late, 2 = early
    "BJ65":            false, // true = 6:5 payout (1.2×), false = 3:2 (1.5×)
    "maxSplit":        4,     // max hands after splitting
    "numDecks":        6,     // decks in shoe
    "deckPen":         52,    // cards remaining at reshuffle (52 = 1-deck cut)
    "numOtherPlayers": 0      // other players at table consuming cards (0–6)
  }
}
```

### Running the CLI

```bash
./bjsim > results.json
```

Output is a JSON object keyed by outcome type, each value being an array of 65
counts indexed by TC bucket (index 0 = TC −8.0, index 32 = TC 0.0,
index 64 = TC +8.0). Progress is written to stderr.

---

## Tests

```bash
./tests/tests
```

Exit code 0 = all pass. Three sections:

**Strategy card** — prints the full basic strategy grid for visual spot-checking
against a published card. No pass/fail.

**Automated tests (432 cases):**

- *Basic strategy (364 cases)* — every hard, soft, and pair cell of the 6-deck
  H17 DAS surrender strategy table.
- *Deviations (52 cases)* — every Illustrious 18 index play tested at its exact
  TC/RC threshold and one step below/above, confirming it fires when it should
  and falls back to basic strategy when it should not.
- *Counting (16 cases)* — Hi-Lo tagging, balanced full-shoe count, and
  `trueCount()` precision at multiple deck depths. Includes a regression test
  for the quarter-deck formula vs the old integer formula:

```
  PASS  Quarter-deck precision differs from old full-deck int formula
        new:4.571429  old:4.000000
```

At 1.75 decks remaining with RC +8, the old formula truncated to TC 4; the
new formula returns 4.57, which crosses integer deviation thresholds and affects
bet sizing boundaries.

Each test prints `PASS` or `FAIL` with the hand, upcard, count, and both the
expected and actual decisions:

```
  PASS  Hard 16 vs 9  TC+4 no-surr → Stand  expected:S  got:S
  PASS  Hard 16 vs 9  TC+3 no-surr → Hit    expected:H  got:H
```

---

## File structure

```
bjsim-cpp/
├── include/
│   ├── config.h         game rules and simulation settings (static globals)
│   ├── hand.h           hand state machine, addCard logic
│   ├── player.h         per-player result accumulator (65×10 bucket array)
│   ├── shoe.h           card shoe, Hi-Lo running/true count
│   ├── strategies.h     basic strategy + Illustrious 18 declarations
│   ├── game.h           runGame / runBatch declarations
│   ├── card.h           card value/suit helpers
│   ├── utilities.h      config loading, handResults enum, debugPrint
│   ├── xoshiro.h        xoshiro256++ PRNG
│   └── json.hpp         nlohmann/json (single-header)
├── public/
│   ├── index.html       single-page web app
│   ├── style.css        dark theme
│   ├── app.js           UI, worker orchestration, EV/RoR math
│   ├── worker.js        Web Worker: loads WASM, runs batch loop
│   ├── bjsim.js         Emscripten output (generated by make wasm)
│   └── bjsim.wasm       Emscripten output (generated by make wasm)
├── tests/
│   └── tests.cpp        full test suite
├── bjsim.cpp            native CLI entry point
├── wasm_api.cpp         Emscripten exported C API
├── game.cpp             core simulation loop (runShoes / runBatch / runGame)
├── player.cpp           result accumulation, quarter-TC bucketing
├── strategies.cpp       basic strategy + I18 deviations
├── shoe.cpp             shuffle, deal, count update
├── utilities.cpp        config parsing, player construction
├── hand.cpp             hand printing
├── card.cpp             card value/print helpers
├── config.json          CLI configuration
├── Makefile             native / wasm / test targets
└── deploy.sh            one-shot Ubuntu 24.04 build + nginx deploy
```
