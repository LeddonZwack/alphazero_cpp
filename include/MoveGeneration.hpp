#ifndef MOVE_GENERATION_HPP
#define MOVE_GENERATION_HPP

#include "State.hpp"  // Defines our State structure
#include <array>
#include <cstdint>
#include <utility>

namespace MoveGeneration {

    constexpr uint64_t NO_A_FILE = 0xFEFEFEFEFEFEFEFEULL;
    constexpr uint64_t NO_H_FILE = 0x7F7F7F7F7F7F7F7FULL;

    constexpr uint8_t WHITE_Q_CASTLE = 0b0001;
    constexpr uint8_t WHITE_K_CASTLE = 0b0010;
    constexpr uint8_t BLACK_Q_CASTLE = 0b0100;
    constexpr uint8_t BLACK_K_CASTLE = 0b1000;


    // Returns a pair: {emptySquares, enemyPieces} based solely on a given pieces array.
    std::pair<uint64_t, uint64_t> getImportantSquares(const std::array<uint64_t, 12> &pieces);

    // Checks if white's king is in check given an updated pieces array.
    // Assumes state is always from white's perspective.
    bool isInCheck(const std::array<uint64_t, 12> &pieces);

    // Generates a valid-move mask (of size 4672) for the current state.
    // Each index in the returned std::array<bool,4672> is true if the move is legal.
    std::pair<std::array<bool, 4672>, bool> getValidMoves(const Chess::State &state);
}

#endif // MOVE_GENERATION_HPP
