#pragma once
#include <cstdint>
#include <limits>

// xoshiro256++ — 4×uint64 state, no reload, ~4-6x faster than MT19937
// Seeded via splitmix64 from a single uint64 seed.
class xoshiro256pp {
public:
    using result_type = uint64_t;
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT64_MAX; }

    explicit xoshiro256pp(uint64_t seed) {
        s[0] = splitmix64(seed);
        s[1] = splitmix64(seed);
        s[2] = splitmix64(seed);
        s[3] = splitmix64(seed);
    }

    result_type operator()() {
        const uint64_t result = rotl(s[0] + s[3], 23) + s[0];
        const uint64_t t = s[1] << 17;
        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];
        s[2] ^= t;
        s[3] = rotl(s[3], 45);
        return result;
    }

    // Lemire's nearly-divisionless bounded random integer in [0, s)
    // Avoids the modulo bias and the division in the common case.
    inline uint64_t bounded_rand(uint64_t s) {
        uint64_t x = operator()();
        __uint128_t m = (__uint128_t)x * s;
        uint64_t l = (uint64_t)m;
        if(l < s) [[unlikely]] {
            uint64_t t = (-s) % s;
            while(l < t) {
                x = operator()();
                m = (__uint128_t)x * s;
                l = (uint64_t)m;
            }
        }
        return (uint64_t)(m >> 64);
    }

private:
    uint64_t s[4];

    static uint64_t rotl(uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    static uint64_t splitmix64(uint64_t& x) {
        x += 0x9e3779b97f4a7c15ULL;
        uint64_t z = x;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }
};
