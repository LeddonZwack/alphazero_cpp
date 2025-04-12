#ifndef BISHOPS_HPP
#define BISHOPS_HPP

#include "SlidingPieces.hpp"

namespace Chess {

    class Bishops : public SlidingPieces {
    public:
        // Constructor. If no bitboard is provided, set the default starting positions based on type.
        // Note: In the original code, the white bishop is set to 0x0000000000000024 and black bishop to 0x2400000000000000.
        explicit Bishops(PieceType t, uint64_t b = 0ULL)
                : SlidingPieces(t, b) {
            if (b == 0ULL) {
                if (t == WHITE_BISHOP) {
                    board = 0x0000000000000024ULL;
                    symbol = "\u265D"; // Unicode bishop (black bishop symbol is used here per original convention)
                } else if (t == BLACK_BISHOP) {
                    board = 0x2400000000000000ULL;
                    symbol = "\u2657"; // Unicode bishop (white bishop symbol used for black in the original)
                }
            }
        }

        // Move-generation: return moves using the sliding bishop logic.
        // This returns a vector of resulting bitboards (each move as a new state).
        inline std::vector<uint64_t> all_moves(uint64_t empty_squares,
                                               uint64_t enemy_pieces) const {
            return SlidingPieces::allBishopMoves(board, empty_squares, enemy_pieces);
        }
    };

} // namespace Chess

#endif // BISHOPS_HPP
