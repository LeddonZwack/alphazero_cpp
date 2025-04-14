#include "MoveGeneration.hpp"
#include "Bitboard.hpp"  // For Bitboard::complement and other helpers.
#include "State.hpp" // (Optional) for WHITE, BLACK, and piece type enums.
#include <vector>
#include <algorithm>

namespace Chess {

    std::pair<uint64_t, uint64_t> getImportantSquares(
            const std::array<BB, 12>& pieces, int color)
    {
        // Determine which pieces belong to the opponent.
        // If the current color is WHITE (1), then the defending (enemy) pieces are BLACK (indices 6..11).
        // Otherwise, if color is BLACK (-1), enemy pieces are WHITE (indices 0..5).
        std::vector<int> defendingPieces;
        if (color == WHITE) {
            defendingPieces = { BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING };
        } else { // color == BLACK
            defendingPieces = { WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING };
        }

        uint64_t occupied = 0ULL;
        uint64_t enemyPieces = 0ULL;
        // Loop over all 12 pieces.
        for (int pt = 0; pt < 12; ++pt) {
            // Combine the bitboards of all pieces to determine occupied squares.
            occupied |= pieces[pt];
            // If the piece type is in the defendingPieces list, add its bitboard.
            if (std::find(defendingPieces.begin(), defendingPieces.end(), pt) != defendingPieces.end()) {
                enemyPieces |= pieces[pt];
            }
        }
        // Empty squares: invert the occupied bitboard (mask to 64 bits using our helper).
        uint64_t emptySquares = Bitboard::complement(occupied);
        return std::make_pair(emptySquares, enemyPieces);
    }

    bool isInCheck(const std::array<BB, 12>& pieces, int color)
    {
        // Determine the king's index and the attacking pieces based on the "attacking" color.
        int attacking_color = -color;
        int kingIndex;
        std::vector<int> attackingPieces;
        if (attacking_color == BLACK) {
            // If the attacker is BLACK then the king under attack is WHITE.
            kingIndex = WHITE_KING;
            attackingPieces = { BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING };
        } else {
            // Otherwise, the attacker is WHITE and the king under attack is BLACK.
            kingIndex = BLACK_KING;
            attackingPieces = { WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING };
        }

        // Retrieve the important squares (empty squares and enemy pieces) for the given perspective.
        auto important = getImportantSquares(pieces, color);
        uint64_t emptySquares = important.first;
        uint64_t enemyPieces  = important.second;

        // For each attacking piece, generate all of its potential moves.
        // We assume that each Piece object has a member function: std::vector<uint64_t> all_moves(uint64_t empty, uint64_t enemy) const;
        for (int pt : attackingPieces) {
            const BB& attacker = pieces[pt];
            std::vector<uint64_t> moves = attacker.all_moves(emptySquares, enemyPieces);
            // If any move overlays the king's current bitboard, the king is in check.
            uint64_t kingBB = pieces[kingIndex];
            for (uint64_t move : moves) {
                if ((move & kingBB) != 0ULL) {
                    return true;
                }
            }
        }
        return false;
    }

} // namespace Chess
