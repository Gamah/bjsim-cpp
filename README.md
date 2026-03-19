# bjsim-cpp

A multi-threaded blackjack simulator for Hi-Lo card counting analysis.
Simulates large numbers of shoes and buckets hand results by true count,
producing edge data comparable to published advantage play tables.

## Building

Requires a C++17 compiler and pthreads.

**Simulator:**
```bash
g++ -O3 -std=c++17 -pthread -o bjsim \
    bjsim.cpp game.cpp player.cpp strategies.cpp \
    shoe.cpp hand.cpp card.cpp utilities.cpp
```

**Tests:**
```bash
g++ -O2 -std=c++17 -o tests/tests \
    tests/tests.cpp hand.cpp shoe.cpp \
    strategies.cpp utilities.cpp card.cpp player.cpp
```

Profiling build (generates `gmon.out` for gprof):
```bash
g++ -O3 -pg -std=c++17 -pthread -o bjsim_prof \
    bjsim.cpp game.cpp player.cpp strategies.cpp \
    shoe.cpp hand.cpp card.cpp utilities.cpp
```

## Configuration

Copy `example-config.json` to `config.json` and edit before running.

```jsonc
{
  "Config": {
    "numThreads": 4,      // parallel simulation threads
    "numShoes": 100000,   // shoes per thread
    "debug": false        // set true for per-hand trace output (single thread only)
  },
  "Rules": {
    "H17":      true,     // dealer hits soft 17
    "DAS":      true,     // double after split allowed
    "RSA":      true,     // resplit aces allowed
    "Surrender": false,   // late surrender offered
    "maxSplit": 4,        // maximum hands after splitting
    "numDecks": 6,        // decks in shoe
    "deckPen":  52        // cards remaining at reshuffle (1 deck = ~17% cut card)
  },
  "Players": [
    { "Name": "Basic",      "Strategy": 1 },
    { "Name": "Deviations", "Strategy": 2 }
  ]
}
```

**Strategies:**

| Value | Description |
|-------|-------------|
| `0`   | Dealer (mirrors dealer rules — useful as a control) |
| `1`   | Basic strategy only |
| `2`   | Basic strategy + Illustrious 18 deviations (Hi-Lo) |

## Running

```bash
./bjsim
```

Results are written to `out.json`, keyed by player name, hand result type,
and true count bucket (-7 through +7).

## Tests

```bash
./tests/tests
```

Exit code `0` = all pass, `1` = failures. Output has three sections:

### Strategy card

Prints the full basic strategy grid at program start — no pass/fail, intended
for visual comparison against a published card (e.g. Blackjack Apprenticeship).

```
  Hard Totals
          2  3  4  5  6  7  8  9  T  A
       ───────────────────────────────
    9 │  H  D  D  D  D  H  H  H  H  H
   10 │  D  D  D  D  D  D  D  D  H  H
   11 │  D  D  D  D  D  D  D  D  D  D   ← doubles vs A in H17 (differs from S17)
   12 │  H  H  S  S  S  H  H  H  H  H
```

Column order is `2 3 4 5 6 7 8 9 T A`.
Symbols: `H`=Hit `S`=Stand `D`=Double `P`=Split `X`=Surrender.

The deviation summary table below the grids lists each index play with its
TC/RC threshold and what basic strategy would do instead.

### Automated tests

Each test prints `PASS` or `FAIL` with a label describing the exact hand,
upcard, count, and expected decision:

```
  PASS  Hard 16 vs 9  TC+4 no-surr → Stand  expected:S  got:S
  PASS  Hard 16 vs 9  TC+3 no-surr → Hit  (below threshold)  expected:H  got:H
  FAIL  Hard 12 vs 2  TC+3 → Stand  expected:S  got:H
```

A `FAIL` means the strategy engine returned a different decision than the
reference value. The label tells you the hand, upcard, true count condition,
and both the expected and actual decision symbols.

**Basic Strategy (364 cases):** Every cell of the hard, soft, and pairs tables.
Expected values are sourced from the 6-deck H17 DAS Surrender strategy card.

**Deviations (52 cases):** Every deviation from the `strategies.cpp` comments
is tested at exactly its TC/RC threshold and one count below/above, confirming
it fires when it should and falls back to basic when it should not.

**Counting (16 cases):** Hi-Lo running count tagging, balanced full-shoe count,
and trueCount precision at multiple deck depths. Includes a regression test
explicitly comparing the old full-deck integer formula against the
quarter-deck float formula:

```
  PASS  Quarter-deck precision differs from old full-deck int formula
        new:4.571429  old:4.000000
```

At 1.75 decks remaining with RC +8, the old formula returned TC 4 (truncated
from full decks); the new formula returns TC 4.57 — a difference that crosses
integer deviation thresholds and affects bet sizing.

### Summary line

```
  Total: 432 passed, 0 failed  ✓ ALL GOOD
```
