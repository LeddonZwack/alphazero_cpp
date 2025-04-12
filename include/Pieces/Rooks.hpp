#ifndef ROOKS_HPP
#define ROOKS_HPP

#include "SlidingPieces.hpp"

namespace Chess {

    class Rooks : public SlidingPieces {
    public:
        // Constructor. If no bitboard is provided, set default initial positions.
        // For white rook: 0x0000000000000081, for black rook: 0x8100000000000000.
        explicit Rooks(PieceType t, uint64_t b = 0ULL)
                : SlidingPieces(t, b) {
            if (b == 0ULL) {
                if (t == WHITE_ROOK) {
                    board = 0x0000000000000081ULL;
                    symbol = "\u265C";
                } else if (t == BLACK_ROOK) {
                    board = 0x8100000000000000ULL;
                    symbol = "\u2656";
                }
            }
        }

        // Move-generation: return moves using the sliding rook logic.
        inline std::vector<uint64_t> all_moves(uint64_t empty_squares,
                                               uint64_t enemy_pieces) const {
            return SlidingPieces::allRookMoves(board, empty_squares, enemy_pieces);
        }
    };

} // namespace Chess

#endif // ROOKS_HPP
