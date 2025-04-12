#ifndef BITBOARD_HPP
#define BITBOARD_HPP

#include <cstdint>
#include <string>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <sstream>

class Bitboard {
public:
    uint64_t board;

    explicit Bitboard(uint64_t b = 0x0000000000000000ULL)
            : board(b) {}

    // Returns the complement (bitwise NOT masked to 64 bits)
    static inline uint64_t complement(uint64_t b) {
        return (~b) & 0xffffffffffffffffULL;
    }

    // Gets the least significant bit set
    static inline uint64_t getLSB(uint64_t b) {
        return b & static_cast<uint64_t>(-static_cast<int64_t>(b));
    }

    // Removes the least significant bit (provided as lsb) from b
    static inline uint64_t removeLSB(uint64_t b, uint64_t lsb) {
        return b & complement(lsb);
    }

    // Converts the bitboard to a 64-character string of 0s and 1s
    static inline std::string toString(uint64_t b) {
        return std::bitset<64>(b).to_string();
    }

    // Reverses the order of bits in the bitboard
    static inline uint64_t reverse(uint64_t b) {
        std::string s = toString(b);
        std::reverse(s.begin(), s.end());
        return std::bitset<64>(s).to_ullong();
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
