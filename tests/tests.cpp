// Blackjack strategy + counting tests
// Build: g++ -O2 -std=c++17 -o tests/tests tests/tests.cpp hand.cpp shoe.cpp strategies.cpp utilities.cpp card.cpp player.cpp
// Run:   ./tests/tests

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
#include <sstream>
#include "../include/utilities.h"
#include "../include/strategies.h"
#include "../include/shoe.h"
#include "../include/card.h"
#include "../include/xoshiro.h"

// ── Test runner ───────────────────────────────────────────────────────────────

struct Suite {
    std::string name;
    int passed = 0, failed = 0;

    void check(bool ok, const std::string& label) {
        if (ok) {
            std::cout << "  PASS  " << label << "\n";
            passed++;
        } else {
            std::cout << "  FAIL  " << label << "\n";
            failed++;
        }
    }

    void summary() const {
        std::string status = failed == 0 ? "OK" : "FAILURES";
        std::cout << "[" << name << "]  " << passed << " passed, "
                  << failed << " failed  " << status << "\n";
    }
};

// ── Hand factory helpers ──────────────────────────────────────────────────────

static hand makeHard(int total, bool canDouble = true, bool canSurrender = true) {
    hand h;
    h.discard();
    h.total      = total;
    h.isSoft     = false;
    h.isPair     = 0;
    h.canDouble  = canDouble;
    h.canSurrender = canSurrender;
    h.numCards   = 2;
    return h;
}

static hand makeSoft(int total, bool canDouble = true) {
    hand h;
    h.discard();
    h.total      = total;
    h.isSoft     = true;
    h.isPair     = 0;
    h.canDouble  = canDouble;
    h.canSurrender = false;
    h.numCards   = 2;
    return h;
}

// Builds a genuine pair via addCard so isPair/canSplit/canDouble are set by the engine.
static hand makePair(int pairValue) {
    hand h;
    h.discard();
    int idx = 0;
    for (int i = 0; i < 13; i++) {
        if (card::value(i) == pairValue) { idx = i; break; }
    }
    h.addCard(idx);
    h.addCard(idx);
    return h;
}

static std::string sym(decisions d) {
    switch (d) {
        case decisions::HIT:       return "H";
        case decisions::STAND:     return "S";
        case decisions::DOUBLE:    return "D";
        case decisions::SPLIT:     return "P";
        case decisions::SURRENDER: return "X";
    }
    return "?";
}

static std::string ucLabel(int uc) {
    if (uc == 10) return "T";
    if (uc == 1)  return "A";
    return std::to_string(uc);
}

// ── Strategy card display ─────────────────────────────────────────────────────
// Prints a visual card matching the BJA 6-deck H17 DAS Surrender layout.
// No pass/fail — intended for human comparison against a published card.

void printStrategyCard() {
    strategies s;
    const int upcards[] = {2,3,4,5,6,7,8,9,10,1};

    std::cout << "══════════════════════════════════════════════════════════════\n";
    std::cout << "  BASIC STRATEGY CARD  (6-deck, H17, DAS, Surrender)\n";
    std::cout << "  H=Hit  S=Stand  D=Double  P=Split  X=Surrender\n";
    std::cout << "══════════════════════════════════════════════════════════════\n\n";

    // Header helper
    auto printHeader = [&]() {
        std::cout << "        ";
        for (int uc : upcards) std::cout << std::setw(3) << ucLabel(uc);
        std::cout << "\n       ───────────────────────────────\n";
    };

    // Hard totals
    std::cout << "  Hard Totals\n";
    printHeader();
    for (int total = 5; total <= 21; total++) {
        std::cout << "  " << std::setw(3) << total << " │";
        for (int uc : upcards) {
            hand h = makeHard(total);
            std::cout << std::setw(3) << sym(s.playerBasic(h, uc));
        }
        std::cout << "\n";
    }

    // Soft totals
    std::cout << "\n  Soft Totals\n";
    printHeader();
    for (int total = 13; total <= 21; total++) {
        // total = 11 (ace) + other card value
        std::string label = "A," + std::to_string(total - 11);
        std::cout << "  " << std::setw(3) << label << " │";
        for (int uc : upcards) {
            hand h = makeSoft(total);
            std::cout << std::setw(3) << sym(s.playerBasic(h, uc));
        }
        std::cout << "\n";
    }

    // Pairs
    std::cout << "\n  Pairs\n";
    printHeader();
    const int    pairVals[]   = {1,2,3,4,5,6,7,8,9,10};
    const char*  pairLabels[] = {"A,A","2,2","3,3","4,4","5,5","6,6","7,7","8,8","9,9","T,T"};
    for (int i = 0; i < 10; i++) {
        std::cout << "  " << std::setw(3) << pairLabels[i] << " │";
        for (int uc : upcards) {
            hand h = makePair(pairVals[i]);
            std::cout << std::setw(3) << sym(s.playerBasic(h, uc));
        }
        std::cout << "\n";
    }

    // Deviation summary table (static — driven by comments in strategies.cpp)
    std::cout << "\n  Deviations from Basic Strategy (Illustrious 18 + others)\n";
    std::cout << "  ──────────────────────────────────────────────────────────\n";
    std::cout << "  Hand    Upcard  Index      Action    (Basic)\n";
    std::cout << "  ──────────────────────────────────────────────────────────\n";
    struct DevRow { const char* hand; const char* up; const char* idx; const char* action; const char* basic; };
    const DevRow devs[] = {
        {"T,T",  "4",  "TC ≥ 6",  "Split",    "Stand" },
        {"T,T",  "5",  "TC ≥ 5",  "Split",    "Stand" },
        {"T,T",  "6",  "TC ≥ 4",  "Split",    "Stand" },
        {"A,8",  "4",  "TC ≥ 3",  "Double",   "Stand" },
        {"A,8",  "5",  "TC ≥ 1",  "Double",   "Stand" },
        {"A,8",  "6",  "RC < 0",  "Stand",    "Double"},
        {"A,6",  "2",  "TC ≥ 1",  "Double",   "Hit"   },
        {"17",   "A",  "H17",     "Surrender","Stand" },
        {"16",   "8",  "TC ≥ 4",  "Surrender","Hit"   },
        {"16",   "9",  "TC ≤ -1", "Hit",      "Surr." },
        {"16",   "T",  "RC ≥ 0",  "Stand",    "Surr." },
        {"16",   "A",  "TC ≥ 3",  "Stand",    "Surr." },
        {"15",   "9",  "TC ≥ 2",  "Surrender","Hit"   },
        {"15",   "T",  "RC < 0",  "Hit",      "Surr." },
        {"15",   "A",  "TC ≤ -1", "Surrender","Hit"   },
        {"15",   "T",  "TC ≥ 4",  "Stand",    "Surr." },
        {"15",   "A",  "TC ≥ 5",  "Stand",    "Hit"   },
        {"13",   "2",  "TC ≤ -1", "Hit",      "Stand" },
        {"12",   "4",  "RC < 0",  "Hit",      "Stand" },
        {"12",   "2",  "TC ≥ 3",  "Stand",    "Hit"   },
        {"12",   "3",  "TC ≥ 2",  "Stand",    "Hit"   },
        {"10",   "T",  "TC ≥ 4",  "Double",   "Hit"   },
        {"10",   "A",  "TC ≥ 3",  "Double",   "Hit"   },
        {"9",    "2",  "TC ≥ 1",  "Double",   "Hit"   },
        {"9",    "7",  "TC ≥ 3",  "Double",   "Hit"   },
        {"8",    "6",  "TC ≥ 2",  "Double",   "Hit"   },
    };
    for (auto& d : devs) {
        std::cout << "  " << std::left
                  << std::setw(7)  << d.hand
                  << " vs " << std::setw(6) << d.up
                  << std::setw(10) << d.idx
                  << "→ " << std::setw(10) << d.action
                  << "(BS: " << d.basic << ")\n";
    }
    std::cout << "\n";
}

// ── Basic strategy tests ──────────────────────────────────────────────────────
// Expected values sourced from BJA 6-deck H17 DAS Surrender strategy card.
// Any FAIL here means the code disagrees with that published strategy.

void testBasicStrategy(Suite& ts) {
    strategies s;

    auto checkBS = [&](hand h, int uc, decisions exp, const std::string& label) {
        decisions got = s.playerBasic(h, uc);
        ts.check(got == exp, label + "  expected:" + sym(exp) + "  got:" + sym(got));
    };

    // ── Hard totals ─────────────────────────────────────────────────────────
    // 5-8: always hit
    for (int t = 5; t <= 8; t++)
        for (int uc : {2,3,4,5,6,7,8,9,10,1})
            checkBS(makeHard(t), uc, decisions::HIT, "Hard " + std::to_string(t) + " vs " + ucLabel(uc));

    // Hard 9: double 3-6, else hit
    for (int uc : {2,7,8,9,10,1})
        checkBS(makeHard(9), uc, decisions::HIT,    "Hard 9 vs " + ucLabel(uc));
    for (int uc : {3,4,5,6})
        checkBS(makeHard(9), uc, decisions::DOUBLE, "Hard 9 vs " + ucLabel(uc));

    // Hard 10: double 2-9, hit vs T/A
    for (int uc : {2,3,4,5,6,7,8,9})
        checkBS(makeHard(10), uc, decisions::DOUBLE, "Hard 10 vs " + ucLabel(uc));
    for (int uc : {10,1})
        checkBS(makeHard(10), uc, decisions::HIT,    "Hard 10 vs " + ucLabel(uc));

    // Hard 11: double vs all upcards in H17 6-deck (including Ace — differs from S17)
    for (int uc : {2,3,4,5,6,7,8,9,10,1})
        checkBS(makeHard(11), uc, decisions::DOUBLE, "Hard 11 vs " + ucLabel(uc));

    // Hard 12: stand 4-6, else hit
    for (int uc : {2,3,7,8,9,10,1})
        checkBS(makeHard(12), uc, decisions::HIT,   "Hard 12 vs " + ucLabel(uc));
    for (int uc : {4,5,6})
        checkBS(makeHard(12), uc, decisions::STAND, "Hard 12 vs " + ucLabel(uc));

    // Hard 13-16: stand 2-6, else hit (with surrender on 15 vs T and 16 vs 9/T/A)
    for (int t : {13,14})
        for (int uc : {2,3,4,5,6,7,8,9,10,1}) {
            decisions exp = (uc >= 2 && uc <= 6) ? decisions::STAND : decisions::HIT;
            checkBS(makeHard(t), uc, exp, "Hard " + std::to_string(t) + " vs " + ucLabel(uc));
        }

    // Hard 15: stand 2-6, surrender vs T, hit vs 7-9/A
    for (int uc : {2,3,4,5,6})
        checkBS(makeHard(15), uc, decisions::STAND,     "Hard 15 vs " + ucLabel(uc));
    checkBS(makeHard(15), 10, decisions::SURRENDER, "Hard 15 vs T");
    for (int uc : {7,8,9,1})
        checkBS(makeHard(15), uc, decisions::HIT,       "Hard 15 vs " + ucLabel(uc));

    // Hard 16: stand 2-6, surrender vs 9/T/A, hit vs 7/8
    for (int uc : {2,3,4,5,6})
        checkBS(makeHard(16), uc, decisions::STAND,     "Hard 16 vs " + ucLabel(uc));
    for (int uc : {9,10,1})
        checkBS(makeHard(16), uc, decisions::SURRENDER, "Hard 16 vs " + ucLabel(uc));
    for (int uc : {7,8})
        checkBS(makeHard(16), uc, decisions::HIT,       "Hard 16 vs " + ucLabel(uc));

    // Hard 17-21: always stand
    for (int t = 17; t <= 21; t++)
        for (int uc : {2,3,4,5,6,7,8,9,10,1})
            checkBS(makeHard(t), uc, decisions::STAND, "Hard " + std::to_string(t) + " vs " + ucLabel(uc));

    // ── Soft totals ──────────────────────────────────────────────────────────
    // Soft 13 (A,2): double 5-6
    for (int uc : {2,3,4,7,8,9,10,1}) checkBS(makeSoft(13), uc, decisions::HIT,    "Soft 13 vs " + ucLabel(uc));
    for (int uc : {5,6})              checkBS(makeSoft(13), uc, decisions::DOUBLE,  "Soft 13 vs " + ucLabel(uc));

    // Soft 14 (A,3): double 5-6
    for (int uc : {2,3,4,7,8,9,10,1}) checkBS(makeSoft(14), uc, decisions::HIT,    "Soft 14 vs " + ucLabel(uc));
    for (int uc : {5,6})              checkBS(makeSoft(14), uc, decisions::DOUBLE,  "Soft 14 vs " + ucLabel(uc));

    // Soft 15 (A,4): double 4-6
    for (int uc : {2,3,7,8,9,10,1})  checkBS(makeSoft(15), uc, decisions::HIT,    "Soft 15 vs " + ucLabel(uc));
    for (int uc : {4,5,6})           checkBS(makeSoft(15), uc, decisions::DOUBLE,  "Soft 15 vs " + ucLabel(uc));

    // Soft 16 (A,5): double 4-6
    for (int uc : {2,3,7,8,9,10,1})  checkBS(makeSoft(16), uc, decisions::HIT,    "Soft 16 vs " + ucLabel(uc));
    for (int uc : {4,5,6})           checkBS(makeSoft(16), uc, decisions::DOUBLE,  "Soft 16 vs " + ucLabel(uc));

    // Soft 17 (A,6): double 3-6, else hit
    for (int uc : {2,7,8,9,10,1})    checkBS(makeSoft(17), uc, decisions::HIT,    "Soft 17 vs " + ucLabel(uc));
    for (int uc : {3,4,5,6})         checkBS(makeSoft(17), uc, decisions::DOUBLE,  "Soft 17 vs " + ucLabel(uc));

    // Soft 18 (A,7): double 2-6, stand 7-8, hit 9/T/A
    for (int uc : {2,3,4,5,6})       checkBS(makeSoft(18), uc, decisions::DOUBLE,  "Soft 18 vs " + ucLabel(uc));
    for (int uc : {7,8})             checkBS(makeSoft(18), uc, decisions::STAND,   "Soft 18 vs " + ucLabel(uc));
    for (int uc : {9,10,1})          checkBS(makeSoft(18), uc, decisions::HIT,     "Soft 18 vs " + ucLabel(uc));

    // Soft 19 (A,8): double vs 6, else stand
    for (int uc : {2,3,4,5,7,8,9,10,1}) checkBS(makeSoft(19), uc, decisions::STAND,  "Soft 19 vs " + ucLabel(uc));
    checkBS(makeSoft(19), 6, decisions::DOUBLE, "Soft 19 vs 6");

    // Soft 20-21: always stand
    for (int t : {20,21})
        for (int uc : {2,3,4,5,6,7,8,9,10,1})
            checkBS(makeSoft(t), uc, decisions::STAND, "Soft " + std::to_string(t) + " vs " + ucLabel(uc));

    // ── Pairs (DAS=true, Surrender=true) ────────────────────────────────────
    // A,A: always split
    for (int uc : {2,3,4,5,6,7,8,9,10,1})
        checkBS(makePair(1), uc, decisions::SPLIT, "A,A vs " + ucLabel(uc));

    // 2,2: split 2-7 (DAS allows 2-3), hit 8-A
    for (int uc : {2,3,4,5,6,7}) checkBS(makePair(2), uc, decisions::SPLIT, "2,2 vs " + ucLabel(uc));
    for (int uc : {8,9,10,1})    checkBS(makePair(2), uc, decisions::HIT,   "2,2 vs " + ucLabel(uc));

    // 3,3: split 2-7, hit 8-A
    for (int uc : {2,3,4,5,6,7}) checkBS(makePair(3), uc, decisions::SPLIT, "3,3 vs " + ucLabel(uc));
    for (int uc : {8,9,10,1})    checkBS(makePair(3), uc, decisions::HIT,   "3,3 vs " + ucLabel(uc));

    // 4,4: split 5-6 (DAS only), else hit
    checkBS(makePair(4), 5, decisions::SPLIT,  "4,4 vs 5");
    checkBS(makePair(4), 6, decisions::SPLIT,  "4,4 vs 6");
    for (int uc : {2,3,4,7,8,9,10,1}) checkBS(makePair(4), uc, decisions::HIT, "4,4 vs " + ucLabel(uc));

    // 5,5: never split — plays as hard 10 (double 2-9, hit T/A)
    for (int uc : {2,3,4,5,6,7,8,9}) checkBS(makePair(5), uc, decisions::DOUBLE, "5,5 vs " + ucLabel(uc));
    for (int uc : {10,1})             checkBS(makePair(5), uc, decisions::HIT,    "5,5 vs " + ucLabel(uc));

    // 6,6: split 2-6 (DAS for 2), hit 7-A
    for (int uc : {2,3,4,5,6}) checkBS(makePair(6), uc, decisions::SPLIT, "6,6 vs " + ucLabel(uc));
    for (int uc : {7,8,9,10,1}) checkBS(makePair(6), uc, decisions::HIT,  "6,6 vs " + ucLabel(uc));

    // 7,7: split 2-7, hit 8-A
    for (int uc : {2,3,4,5,6,7}) checkBS(makePair(7), uc, decisions::SPLIT, "7,7 vs " + ucLabel(uc));
    for (int uc : {8,9,10,1})    checkBS(makePair(7), uc, decisions::HIT,   "7,7 vs " + ucLabel(uc));

    // 8,8: always split
    for (int uc : {2,3,4,5,6,7,8,9,10,1})
        checkBS(makePair(8), uc, decisions::SPLIT, "8,8 vs " + ucLabel(uc));

    // 9,9: split vs 2-6/8-9, stand vs 7/T/A
    for (int uc : {2,3,4,5,6,8,9}) checkBS(makePair(9), uc, decisions::SPLIT, "9,9 vs " + ucLabel(uc));
    for (int uc : {7,10,1})        checkBS(makePair(9), uc, decisions::STAND, "9,9 vs " + ucLabel(uc));

    // T,T: never split — plays as hard 20, always stand
    for (int uc : {2,3,4,5,6,7,8,9,10,1})
        checkBS(makePair(10), uc, decisions::STAND, "T,T vs " + ucLabel(uc));

    // ── Dealer strategy (H17) ────────────────────────────────────────────────
    strategies sd;
    {
        hand dh = makeHard(16);
        ts.check(sd.dealer(dh) == decisions::HIT,   "Dealer hard 16 → Hit");
    }
    {
        hand dh = makeHard(17);
        ts.check(sd.dealer(dh) == decisions::STAND, "Dealer hard 17 → Stand (H17)");
    }
    {
        // Soft 17: hit in H17 game
        hand dh = makeSoft(17);
        ts.check(sd.dealer(dh) == decisions::HIT,   "Dealer soft 17 → Hit (H17)");
    }
    {
        hand dh = makeSoft(18);
        ts.check(sd.dealer(dh) == decisions::STAND, "Dealer soft 18 → Stand");
    }
}

// ── Deviation tests ───────────────────────────────────────────────────────────
// Each test checks the boundary of a specific deviation:
//   - at exactly the TC threshold (should deviate)
//   - one count below/above the threshold (should fall back to basic)

void testDeviations(Suite& ts) {
    strategies s;

    auto check = [&](hand h, int uc, int tc, int rc, decisions exp, const std::string& label) {
        decisions got = s.playerDeviations(h, uc, tc, rc);
        ts.check(got == exp, label + "  expected:" + sym(exp) + "  got:" + sym(got));
    };

    // ── Pair of tens ─────────────────────────────────────────────────────────
    check(makePair(10), 4,  6, 0, decisions::SPLIT, "T,T vs 4  TC+6  → Split");
    check(makePair(10), 4,  5, 0, decisions::STAND, "T,T vs 4  TC+5  → Stand (below threshold)");
    check(makePair(10), 5,  5, 0, decisions::SPLIT, "T,T vs 5  TC+5  → Split");
    check(makePair(10), 5,  4, 0, decisions::STAND, "T,T vs 5  TC+4  → Stand (below threshold)");
    check(makePair(10), 6,  4, 0, decisions::SPLIT, "T,T vs 6  TC+4  → Split");
    check(makePair(10), 6,  3, 0, decisions::STAND, "T,T vs 6  TC+3  → Stand (below threshold)");

    // ── Soft 19 (A,8) ────────────────────────────────────────────────────────
    check(makeSoft(19), 4,  3, 0, decisions::DOUBLE, "A,8 vs 4  TC+3  → Double");
    check(makeSoft(19), 4,  2, 0, decisions::STAND,  "A,8 vs 4  TC+2  → Stand (below threshold)");
    check(makeSoft(19), 5,  1, 0, decisions::DOUBLE, "A,8 vs 5  TC+1  → Double");
    check(makeSoft(19), 5,  0, 0, decisions::STAND,  "A,8 vs 5  TC 0  → Stand (below threshold)");
    // vs 6: basic doubles, but suppress double when RC < 0
    check(makeSoft(19), 6,  0, -1, decisions::STAND,  "A,8 vs 6  RC-1  → Stand (suppress basic double)");
    check(makeSoft(19), 6,  0,  0, decisions::DOUBLE, "A,8 vs 6  RC 0  → Double (basic)");

    // ── Soft 17 (A,6) ────────────────────────────────────────────────────────
    check(makeSoft(17), 2,  1, 0, decisions::DOUBLE, "A,6 vs 2  TC+1  → Double");
    check(makeSoft(17), 2,  0, 0, decisions::HIT,    "A,6 vs 2  TC 0  → Hit (below threshold)");

    // ── Hard 17 vs Ace (H17 surrender) ───────────────────────────────────────
    check(makeHard(17, true, true), 1, 0, 0, decisions::SURRENDER, "Hard 17 vs A  H17  → Surrender");

    // ── Hard 16 ──────────────────────────────────────────────────────────────
    // vs 9: stand TC4+ when no surrender; hit TC-1 and below when surrender available
    check(makeHard(16, true, false),  9,  4, 0, decisions::STAND, "Hard 16 vs 9  TC+4 no-surr → Stand");
    check(makeHard(16, true, false),  9,  3, 0, decisions::HIT,   "Hard 16 vs 9  TC+3 no-surr → Hit  (below threshold)");
    // vs T: stand when RC >= 0 and no surrender
    check(makeHard(16, true, false), 10,  0,  1, decisions::STAND, "Hard 16 vs T  RC+1 no-surr → Stand");
    check(makeHard(16, true, false), 10,  0, -1, decisions::HIT,   "Hard 16 vs T  RC-1 no-surr → Hit  (negative RC)");
    // vs A: stand TC3+ and no surrender
    check(makeHard(16, true, false),  1,  3, 0, decisions::STAND, "Hard 16 vs A  TC+3 no-surr → Stand");
    check(makeHard(16, true, false),  1,  2, 0, decisions::HIT,   "Hard 16 vs A  TC+2 no-surr → Hit  (below threshold)");
    // vs 8: surrender TC4+ (deviation adds surrender where basic says hit)
    check(makeHard(16, true, true),   8,  4, 0, decisions::SURRENDER, "Hard 16 vs 8  TC+4 → Surrender");
    check(makeHard(16, true, true),   8,  3, 0, decisions::HIT,       "Hard 16 vs 8  TC+3 → Hit  (below threshold)");
    // vs 9: hit when TC <= -1 (overrides basic surrender)
    check(makeHard(16, true, true),   9, -1, 0, decisions::HIT,       "Hard 16 vs 9  TC-1 → Hit  (override surrender)");

    // ── Hard 15 ──────────────────────────────────────────────────────────────
    // vs T: stand TC4+ no surrender
    check(makeHard(15, true, false), 10,  4, 0, decisions::STAND, "Hard 15 vs T  TC+4 no-surr → Stand");
    check(makeHard(15, true, false), 10,  3, 0, decisions::HIT,   "Hard 15 vs T  TC+3 no-surr → Hit  (below threshold)");
    // vs A: stand TC5+ no surrender
    check(makeHard(15, true, false),  1,  5, 0, decisions::STAND, "Hard 15 vs A  TC+5 no-surr → Stand");
    check(makeHard(15, true, false),  1,  4, 0, decisions::HIT,   "Hard 15 vs A  TC+4 no-surr → Hit  (below threshold)");
    // vs 9: surrender TC2+
    check(makeHard(15, true, true),   9,  2, 0, decisions::SURRENDER, "Hard 15 vs 9  TC+2 → Surrender");
    check(makeHard(15, true, true),   9,  1, 0, decisions::HIT,       "Hard 15 vs 9  TC+1 → Hit  (below threshold)");
    // vs T: hit when RC < 0 (overrides basic surrender)
    check(makeHard(15, true, true),  10,  0, -1, decisions::HIT,       "Hard 15 vs T  RC-1 → Hit  (override surrender)");
    check(makeHard(15, true, true),  10,  0,  0, decisions::SURRENDER, "Hard 15 vs T  RC 0 → Surrender (basic)");
    // vs A: surrender TC <= -1
    check(makeHard(15, true, true),   1, -1, 0, decisions::SURRENDER, "Hard 15 vs A  TC-1 → Surrender");
    check(makeHard(15, true, true),   1,  0, 0, decisions::HIT,       "Hard 15 vs A  TC 0 → Hit  (below threshold)");

    // ── Hard 13 ──────────────────────────────────────────────────────────────
    check(makeHard(13),  2, -1, 0, decisions::HIT,   "Hard 13 vs 2  TC-1 → Hit  (override stand)");
    check(makeHard(13),  2,  0, 0, decisions::STAND, "Hard 13 vs 2  TC 0 → Stand (basic)");

    // ── Hard 12 ──────────────────────────────────────────────────────────────
    check(makeHard(12),  2,  3, 0, decisions::STAND, "Hard 12 vs 2  TC+3 → Stand");
    check(makeHard(12),  2,  2, 0, decisions::HIT,   "Hard 12 vs 2  TC+2 → Hit  (below threshold)");
    check(makeHard(12),  3,  2, 0, decisions::STAND, "Hard 12 vs 3  TC+2 → Stand");
    check(makeHard(12),  3,  1, 0, decisions::HIT,   "Hard 12 vs 3  TC+1 → Hit  (below threshold)");
    // vs 4: hit when RC < 0 (overrides basic stand)
    check(makeHard(12),  4,  0, -1, decisions::HIT,   "Hard 12 vs 4  RC-1 → Hit  (override stand)");
    check(makeHard(12),  4,  0,  0, decisions::STAND, "Hard 12 vs 4  RC 0 → Stand (basic)");

    // ── Hard 10 ──────────────────────────────────────────────────────────────
    check(makeHard(10), 10,  4, 0, decisions::DOUBLE, "Hard 10 vs T  TC+4 → Double");
    check(makeHard(10), 10,  3, 0, decisions::HIT,    "Hard 10 vs T  TC+3 → Hit  (below threshold)");
    check(makeHard(10),  1,  3, 0, decisions::DOUBLE, "Hard 10 vs A  TC+3 → Double");
    check(makeHard(10),  1,  2, 0, decisions::HIT,    "Hard 10 vs A  TC+2 → Hit  (below threshold)");

    // ── Hard 9 ───────────────────────────────────────────────────────────────
    check(makeHard(9),   2,  1, 0, decisions::DOUBLE, "Hard 9  vs 2  TC+1 → Double");
    check(makeHard(9),   2,  0, 0, decisions::HIT,    "Hard 9  vs 2  TC 0 → Hit  (below threshold)");
    check(makeHard(9),   7,  3, 0, decisions::DOUBLE, "Hard 9  vs 7  TC+3 → Double");
    check(makeHard(9),   7,  2, 0, decisions::HIT,    "Hard 9  vs 7  TC+2 → Hit  (below threshold)");

    // ── Hard 8 ───────────────────────────────────────────────────────────────
    check(makeHard(8),   6,  2, 0, decisions::DOUBLE, "Hard 8  vs 6  TC+2 → Double");
    check(makeHard(8),   6,  1, 0, decisions::HIT,    "Hard 8  vs 6  TC+1 → Hit  (below threshold)");
}

// ── Counting tests ────────────────────────────────────────────────────────────

void testCounting(Suite& ts) {
    xoshiro256pp rng(42);

    // ── Hi-Lo tagging ─────────────────────────────────────────────────────────
    // Low  cards (2-6):  +1 each
    // High cards (T/A):  -1 each
    // Neutral (7-8-9):    0
    {
        shoe s;
        s.cards.clear();
        s.runningCount = 0;

        s.cards.push_back(1); s.getCard();  // index 1 = value 2 (low)
        ts.check(s.runningCount == 1,  "Deal 2: RC = +1");

        s.cards.push_back(9); s.getCard();  // index 9 = value 10 (high)
        ts.check(s.runningCount == 0,  "Deal T: RC back to 0");

        s.cards.push_back(0); s.getCard();  // index 0 = value 1/Ace (high)
        ts.check(s.runningCount == -1, "Deal A: RC = -1");

        s.cards.push_back(6); s.getCard();  // index 6 = value 7 (neutral)
        ts.check(s.runningCount == -1, "Deal 7: RC unchanged");

        s.runningCount = 0;
        for (int idx : {1,2,3,4,5}) {       // values 2,3,4,5,6 — all low
            s.cards.push_back(idx);
            s.getCard();
        }
        ts.check(s.runningCount == 5, "Deal 2-3-4-5-6: RC = +5");
    }

    // ── Full shoe balanced count ──────────────────────────────────────────────
    // Any complete shoe must end at RC=0 (symmetric Hi-Lo tag distribution).
    {
        shoe s;
        s.shuffleCards(rng);
        int sz = (int)s.cards.size();
        ts.check(sz == 52 * config::rules::numDecks,
                 "Fresh shoe size = " + std::to_string(sz) + " cards");
        while (!s.cards.empty()) s.getCard();
        ts.check(s.runningCount == 0, "Full shoe dealt: RC returns to 0");
    }

    // ── trueCount quarter-deck precision ─────────────────────────────────────
    // Formula: (RC * 4) / ceil(cardsLeft / 13)
    // We set up controlled shoe states and check the float result.

    auto tcTest = [&](int rc, int cardsLeft, float expected, const std::string& label) {
        shoe s;
        s.runningCount = rc;
        s.cards.clear();
        for (int i = 0; i < cardsLeft; i++) s.cards.push_back(i % 52);
        float got = s.trueCount();
        ts.check(std::fabs(got - expected) < 0.001f,
                 label + "  expected:" + std::to_string(expected) + "  got:" + std::to_string(got));
    };

    // RC=+8, exactly 2 decks left (8 quarter-decks): TC = 32/8 = 4.000
    tcTest(8, 104, 4.0f,
           "TC: RC+8, 104 cards (2.00 decks, 8 qd)");

    // RC=+8, 1.75 decks left (7 quarter-decks): TC = 32/7 ≈ 4.571
    tcTest(8, 91, (8.0f * 4) / 7,
           "TC: RC+8,  91 cards (1.75 decks, 7 qd)");

    // RC=+8, 1.5 decks left (6 quarter-decks): TC = 32/6 ≈ 5.333
    tcTest(8, 78, (8.0f * 4) / 6,
           "TC: RC+8,  78 cards (1.50 decks, 6 qd)");

    // RC=+8, 1.25 decks left (5 quarter-decks): TC = 32/5 = 6.400
    tcTest(8, 65, (8.0f * 4) / 5,
           "TC: RC+8,  65 cards (1.25 decks, 5 qd)");

    // RC=+4, 1 quarter-deck left (13 cards): TC = 16/1 = 16.000
    tcTest(4, 13, 16.0f,
           "TC: RC+4,  13 cards (0.25 decks, 1 qd)");

    // RC=+3, 1 card left — still rounds up to 1 quarter-deck: TC = 12/1 = 12.000
    tcTest(3, 1, 12.0f,
           "TC: RC+3,   1 card  (rounds to 1 qd)");

    // RC=-6, 2 decks: TC = -24/8 = -3.000
    tcTest(-6, 104, -3.0f,
           "TC: RC-6, 104 cards (2.00 decks, negative count)");

    // ── Demonstrate old integer bug ───────────────────────────────────────────
    // Old formula: decksLeft = ((cardsLeft-1)/52)+1, TC = RC/decksLeft (int div)
    // At RC=+8, 91 cards: oldDecksLeft=2, oldTC=4 — but correct TC≈4.57
    {
        shoe s;
        s.runningCount = 8;
        s.cards.clear();
        for (int i = 0; i < 91; i++) s.cards.push_back(i % 52);

        float newTC   = s.trueCount();
        int   oldDecks = ((91 - 1) / 52) + 1;   // = 2
        float oldTC    = (float)(8 / oldDecks);  // = 4.0

        ts.check(std::fabs(newTC - oldTC) > 0.1f,
                 "Quarter-deck precision differs from old full-deck int formula"
                 "  new:" + std::to_string(newTC) + "  old:" + std::to_string(oldTC));

        // Both truncate to same integer bucket at TC 4-5, but the float value
        // is meaningfully higher and will cross integer thresholds sooner.
        ts.check(newTC > oldTC,
                 "New TC (" + std::to_string(newTC) + ") > old TC (" +
                 std::to_string(oldTC) + ") — quarter-deck gives higher true count");
    }
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main() {
    config::settings::debug  = false;
    config::rules::H17       = true;
    config::rules::DAS       = true;
    config::rules::RSA       = true;
    config::rules::Surrender = true;
    config::rules::maxSplit  = 4;
    config::rules::numDecks  = 6;
    config::rules::deckPen   = 52;

    printStrategyCard();

    Suite bs{"Basic Strategy"};
    std::cout << "── Basic Strategy ────────────────────────────────────────────\n";
    testBasicStrategy(bs);
    bs.summary();

    Suite dev{"Deviations"};
    std::cout << "\n── Deviations ────────────────────────────────────────────────\n";
    testDeviations(dev);
    dev.summary();

    Suite cnt{"Counting"};
    std::cout << "\n── Counting ──────────────────────────────────────────────────\n";
    testCounting(cnt);
    cnt.summary();

    int totalFailed = bs.failed + dev.failed + cnt.failed;
    int totalPassed = bs.passed + dev.passed + cnt.passed;
    std::cout << "\n══════════════════════════════════════════════════════════════\n";
    std::cout << "  Total: " << totalPassed << " passed, " << totalFailed << " failed";
    std::cout << (totalFailed == 0 ? "  ✓ ALL GOOD\n" : "  ✗ FAILURES\n");
    std::cout << "══════════════════════════════════════════════════════════════\n";

    return totalFailed > 0 ? 1 : 0;
}
