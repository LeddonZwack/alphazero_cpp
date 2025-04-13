#ifndef BITBOARD_HPP
#define BITBOARD_HPP

#include <cstdint>
#include <string>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>     // for throw

class Bitboard {
public:
    uint64_t board;

    explicit Bitboard(uint64_t b = 0ULL)
            : board(b) {}

    // Returns the complement (bitwise NOT masked to 64 bits)
    static inline uint64_t complement(uint64_t b) {
        return (~b) & 0xffffffffffffffffULL;
    }

    // Gets the least significant bit set (same as x & -x).
    static inline uint64_t getLSB(uint64_t b) {
        return b & static_cast<uint64_t>(-static_cast<int64_t>(b));
    }

    // Removes the least significant bit (provided as lsb) from b
    static inline uint64_t removeLSB(uint64_t b, uint64_t lsb) {
        return b & complement(lsb);
    }

    // Converts the bitboard to a 64-character string of 0s and 1s (unoptimized but convenient).
    static inline std::string toString(uint64_t b) {
        return std::bitset<64>(b).to_string();
    }

    // Count bits set (popcount).
    static inline int popcount(uint64_t b) {
    #ifdef __GNUC__  // GCC, Clang, ICC
        return __builtin_popcountll(b);
    #elif _MSC_VER    // MSVC
        // Use hardware instruction if available; fallback if older compiler:
        return __popcnt64(b);
        #else
            // Fallback generic approach:
            // Brian Kernighan’s method
            int count = 0;
            while (b) {
                b &= (b - 1);
                ++count;
            }
            return count;
    #endif
    }

    // Return index (0-63) of least significant set bit. Throw if b=0.
    static inline int lsb_index(uint64_t b) {
        if (b == 0ULL) {
            throw std::runtime_error("lsb_index() called on 0 bitboard!");
        }
    #ifdef __GNUC__
        // "count trailing zeros"
        return __builtin_ctzll(b);
    #elif _MSC_VER
        // On MSVC, use _BitScanForward64 or a fallback
        unsigned long idx;
        _BitScanForward64(&idx, b);
        return static_cast<int>(idx);
    #else
        // Fallback naive approach
        // scan from 0..63
        int idx = 0;
        while ((b & 1) == 0) {
            b >>= 1;
            ++idx;
        }
        return idx;
    #endif
    }

    // Reverses the order of bits in the bitboard (bitwise approach, no strings).
    static inline uint64_t reverse(uint64_t b) {
        // Standard 64-bit bit-reversal
        b = ((b & 0x5555555555555555ULL) << 1)  | ((b >> 1)  & 0x5555555555555555ULL);
        b = ((b & 0x3333333333333333ULL) << 2)  | ((b >> 2)  & 0x3333333333333333ULL);
        b = ((b & 0x0f0f0f0f0f0f0f0fULL) << 4)  | ((b >> 4)  & 0x0f0f0f0f0f0f0f0fULL);
        b = ((b & 0x00ff00ff00ff00ffULL) << 8)  | ((b >> 8)  & 0x00ff00ff00ff00ffULL);
        b = ((b & 0x0000ffff0000ffffULL) << 16) | ((b >> 16) & 0x0000ffff0000ffffULL);
        b = (b << 32) | (b >> 32);
        return b;
    }

    // Prints the bitboard in an 8×8 grid with an optional message
    static inline void print(uint64_t b, const std::string& msg = "") {
        std::string s = toString(b);
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
};

#endif // BITBOARD_HPP
