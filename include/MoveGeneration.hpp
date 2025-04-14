#ifndef MOVE_GENERATION_HPP
#define MOVE_GENERATION_HPP

#include "State.hpp"     // Contains the definition of State and the Piece objects array
#include "Bitboard.hpp"  // Contains our helper functions (e.g. complement)
#include <array>
#include <cstdint>
#include <vector>
#include <utility>

namespace Chess {

    /**
     * @brief Returns a pair of bitboards:
     *        - first: The bitboard of empty squares.
     *        - second: The bitboard representing enemy pieces.
     *
     * @param pieces The array of 12 Piece objects representing the current state.
     * @param color  The player's color (WHITE is 1, BLACK is -1). In this context,
     *               the defending pieces are those opposite to the current color.
     * @return std::pair<uint64_t, uint64_t> pair(emptySquares, enemyPieces)
     */
    std::pair<uint64_t, uint64_t> getImportantSquares(
            const std::array<BB, 12>& pieces, int color);

    /**
     * @brief Determines if the king of the given color is in check.
     *
     * For each attacking piece (determined from the opposite color), this function
     * generates all potential moves (using Piece::all_moves) and checks if any move
     * can capture the king.
     *
     * @param pieces The current state's pieces.
     * @param color  The perspective of the player whose king is to be checked.
     * @return true if the king is in check; false otherwise.
     */
    bool isInCheck(const std::array<BB, 12>& pieces, int color);

} // namespace Chess

#endif // MOVE_GENERATION_HPP
