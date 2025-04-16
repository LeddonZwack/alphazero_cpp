#include "GameStatus.hpp"
#include "State.hpp"
#include "MoveGeneration.hpp"
#include "bitboard/bitboard_utils.hpp"
#include <algorithm>

// For counting empty squares in state.typeAtSquare.
static inline int countEmptySquares(const std::array<uint8_t, 64> &typeAtSquare) {
    int count = 0;
    for (uint8_t sq : typeAtSquare) {
        if (sq == bb::NO_PIECE)
            ++count;
    }
    return count;
}

namespace GameStatus {

    std::pair<int, bool> evaluateState(const Chess::State &state,
                                       const std::array<bool, 4672> *valid_moves_ptr) {

        std::cout << "evaluateState CALLED @ " << __FILE__ << ":" << __LINE__ << std::endl;
        // If no valid_moves array is provided, generate it.
        std::array<bool, 4672> valid_moves;
        if (valid_moves_ptr == nullptr) {
            valid_moves = MoveGeneration::getValidMoves(state);
        } else {
            valid_moves = *valid_moves_ptr;
        }

        // Terminal condition checks:
        // 1. Repetition: if repeated_state flag's second bit is on.
        if ((state.flags.repeated_state & 0b10) != 0) {
            return {0, true};
        }
        // 2. Fifty-move rule.
        if (state.flags.half_move_count >= 50) {
            return {0, true};
        }

        // 3. Insufficient material:
        int num_empty = countEmptySquares(state.typeAtSquare);
        if (num_empty == 62) {
            // Only two kings remain.
            return {0, true};
        }
        if (num_empty == 61) {
            // Only one extra piece exists – if it's a knight or bishop.
            // Check white and black knights and bishops via their bitboards.
            if ((state.pieces[bb::WHITE_BISHOP] | state.pieces[bb::BLACK_BISHOP]) ||
                (state.pieces[bb::WHITE_KNIGHT] | state.pieces[bb::BLACK_KNIGHT])) {
                return {0, true};
            }
        }
        if (num_empty == 60) {
            // King and same-color bishops case:
            if (state.pieces[bb::WHITE_BISHOP] && state.pieces[bb::BLACK_BISHOP]) {
                // Determine bishop square colors.
                // Use a white-square mask: bits set on white squares.
                constexpr uint64_t WHITE_SQUARE_MASK = 0xaa55aa55aa55aa55ULL;
                int wb_index = bb_utils::ctz(state.pieces[bb::WHITE_BISHOP]);
                int bb_index = bb_utils::ctz(state.pieces[bb::BLACK_BISHOP]);
                bool wb_on_white = ((WHITE_SQUARE_MASK >> wb_index) & 1ULL) != 0;
                bool bb_on_white = ((WHITE_SQUARE_MASK >> bb_index) & 1ULL) != 0;
                if (wb_on_white == bb_on_white) {
                    return {0, true};
                }
            }
        }

        // 4. Legal move availability:
        bool hasLegalMove = std::any_of(valid_moves.begin(), valid_moves.end(),
                                        [](bool mv) { return mv; });
        if (!hasLegalMove) {
            // No legal moves: determine if it's a checkmate or stalemate.
            if (MoveGeneration::isInCheck(state.pieces)) {
                return {1, true};  // Checkmate: White wins.
            } else {
                return {0, true};  // Stalemate or draw.
            }
        }

        // If we pass all these, the state is not terminal.
        return {0, false};
    }

} // namespace GameStatus
