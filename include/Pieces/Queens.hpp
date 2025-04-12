#ifndef QUEENS_HPP
#define QUEENS_HPP

#include "SlidingPieces.hpp"

namespace Chess {

    class Queens : public SlidingPieces {
    public:
        // Constructor. If no bitboard is provided, set default positions.
        // For white queen: 0x0000000000000010, for black queen: 0x1000000000000000.
        explicit Queens(PieceType t, uint64_t b = 0ULL)
                : SlidingPieces(t, b) {
            if (b == 0ULL) {
                if (t == WHITE_QUEEN) {
                    board = 0x0000000000000010ULL;
                    symbol = "\u265B";
                } else if (t == BLACK_QUEEN) {
                    board = 0x1000000000000000ULL;
                    symbol = "\u2655";
                }
            }
        }

        // Move-generation for Queen: combine rook and bishop moves.
        inline std::vector<uint64_t> all_moves(uint64_t empty_squares,
                                               uint64_t enemy_pieces) const {
            // Get rook moves (horizontal & vertical)
            std::vector<uint64_t> rookMoves = SlidingPieces::allRookMoves(board, empty_squares, enemy_pieces);
            // Get bishop moves (diagonals)
            std::vector<uint64_t> bishopMoves = SlidingPieces::allBishopMoves(board, empty_squares, enemy_pieces);
            // Combine the two vectors.
            rookMoves.insert(rookMoves.end(), bishopMoves.begin(), bishopMoves.end());
            return rookMoves;
        }
    };

} // namespace Chess

#endif // QUEENS_HPP
