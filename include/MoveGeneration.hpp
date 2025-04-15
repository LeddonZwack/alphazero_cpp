#ifndef MOVE_GENERATION_HPP
#define MOVE_GENERATION_HPP

#include "State.hpp"  // Defines our State structure
#include <array>
#include <cstdint>
#include <utility>

namespace MoveGeneration {

    constexpr uint64_t FILE_A_MASK = 0x0101010101010101ULL; // left edge
    constexpr uint64_t FILE_H_MASK = 0x8080808080808080ULL; // right edge

    // Returns a pair: {emptySquares, enemyPieces} based solely on a given pieces array.
    std::pair<uint64_t, uint64_t> getImportantSquares(const std::array<uint64_t, 12> &pieces);

    // Checks if white's king is in check given an updated pieces array.
    // Assumes state is always from white's perspective.
    bool isInCheck(const std::array<uint64_t, 12> &pieces);

    // Generates a valid-move mask (of size 4672) for the current state.
    // Each index in the returned std::array<bool,4672> is true if the move is legal.
    std::array<bool, 4672> getValidMoves(const Chess::State &state);
}

#endif // MOVE_GENERATION_HPP
