#ifndef BITBOARD_UTILS_H
#define BITBOARD_UTILS_H

#include <cstdint>
#include <string>
#include <bitset>
#include <sstream>
#include <iostream>
#include <stdexcept>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace bb_utils {

    // Returns the bitwise complement (NOT) of b
    static inline uint64_t complement(uint64_t b) {
        return ~b;
    }

    // Returns the least significant bit (LSB) set in b
    static inline uint64_t lsb(uint64_t b) {
        return b & static_cast<uint64_t>(-static_cast<int64_t>(b));
    }

    // Removes and returns the LSB from b
    static inline uint64_t pop_lsb(uint64_t &b) {
        uint64_t l = lsb(b);
        b &= complement(l);
        return l;
    }

    // Population count (number of bits set)
    #ifdef __GNUC__
    static inline int popcount(uint64_t b) {
        return __builtin_popcountll(b);
    }
    static inline int ctz(uint64_t b) {
        return __builtin_ctzll(b);
    }
    #elif _MSC_VER
        static inline int popcount(uint64_t b) {
        return static_cast<int>(__popcnt64(b));
    }
    static inline int ctz(uint64_t b) {
        unsigned long idx;
        if (!_BitScanForward64(&idx, b)) {
            throw std::runtime_error("ctz(0) called on zero bitboard");
        }
        return static_cast<int>(idx);
    }
    #else
    static inline int popcount(uint64_t b) {
        int count = 0;
        while (b) {
            b &= (b - 1);
            ++count;
        }
        return count;
    }
    static inline int ctz(uint64_t b) {
        if (b == 0) throw std::runtime_error("ctz(0) called on zero bitboard");
        int idx = 0;
        while ((b & 1ULL) == 0ULL) {
            b >>= 1;
            ++idx;
        }
        return idx;
    }
    #endif

    // Convert bitboard to 64-character string
    static inline std::string to_string(uint64_t b) {
        return std::bitset<64>(b).to_string();
    }

    // Reverse bits in the bitboard
    static inline uint64_t reverse(uint64_t b) {
        b = ((b & 0x5555555555555555ULL) << 1)  | ((b >> 1)  & 0x5555555555555555ULL);
        b = ((b & 0x3333333333333333ULL) << 2)  | ((b >> 2)  & 0x3333333333333333ULL);
        b = ((b & 0x0f0f0f0f0f0f0f0fULL) << 4)  | ((b >> 4)  & 0x0f0f0f0f0f0f0f0fULL);
        b = ((b & 0x00ff00ff00ff00ffULL) << 8)  | ((b >> 8)  & 0x00ff00ff00ff00ffULL);
        b = ((b & 0x0000ffff0000ffffULL) << 16) | ((b >> 16) & 0x0000ffff0000ffffULL);
        return (b << 32) | (b >> 32);
    }

    // Print the bitboard as a 8x8 grid with an optional message
    static inline void print(uint64_t b, const std::string &msg = "") {
        std::string s = to_string(b);
        std::ostringstream oss;
        oss << "\nBitboard: " << msg << "\n";
        for (int row = 0; row < 8; ++row) {
            oss << s.substr(row * 8, 4)
                << " "
                << s.substr(row * 8 + 4, 4)
                << "\n";
        }
        std::cout << oss.str();
    }

    // Placeholder for future bitboard utilities (e.g., magic bitboard helpers)
    // TODO: add mask, magic index, PEXT/PDEP intrinsics here

} // namespace bb_utils

#endif // BITBOARD_UTILS_H
