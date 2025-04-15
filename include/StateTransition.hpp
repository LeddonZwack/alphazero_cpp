#ifndef STATE_TRANSITION_HPP
#define STATE_TRANSITION_HPP

#include "State.hpp"
#include "MoveMapping.hpp"
#include <cstdint>

namespace StateTransition {

    /// Doesn't update repeated_state flag
    // Apply an action to a given state and return a new state.
    // This function is pure: it does not alter the original state.
    // The action integer is decoded as follows:
    //   fromSquare = action % 64
    //   moveType    = action / 64
    // The function handles edge cases:
    //   - Castling (updates rook positions)
    //   - Captures (clears opponent bit from its bitboard)
    //   - En passant (special pawn captures)
    //   - Promotions (replacing a pawn with a promoted piece)
    // When an edge-case rule applies, the appropriate bitboards are updated.
    Chess::State getNextState(const Chess::State &currState, int action);

    // A temporary function that applies an action to a copy of piece bitboards.
    // Useful for move-generation and checking king safety.
    std::array<uint64_t, 12> tempApplyActionToPieces(const Chess::State &currState, int action);

    // --- Perspective / State Transformation Functions ---

    /// Flips a bitboard 180° by reversing the bits.
    /// This is equivalent to a vertical and horizontal flip.
    inline uint64_t flip180(uint64_t bb) {
        // Assumes bb_utils::reverse is defined in BitboardUtils.hpp.
        return bb_utils::reverse(bb);
    }

    /// Flips the piece type from white to black and vice versa.
    /// Uses our standard PieceType mapping.
    inline int flipPieceType(int pieceType) {
        switch (pieceType) {
            case 0:  return 6;   // WHITE_PAWN -> BLACK_PAWN
            case 1:  return 7;   // WHITE_KNIGHT -> BLACK_KNIGHT
            case 2:  return 8;   // WHITE_BISHOP -> BLACK_BISHOP
            case 3:  return 9;   // WHITE_ROOK -> BLACK_ROOK
            case 4:  return 10;  // WHITE_QUEEN -> BLACK_QUEEN
            case 5:  return 11;  // WHITE_KING -> BLACK_KING
            case 6:  return 0;   // BLACK_PAWN -> WHITE_PAWN
            case 7:  return 1;   // BLACK_KNIGHT -> WHITE_KNIGHT
            case 8:  return 2;   // BLACK_BISHOP -> WHITE_BISHOP
            case 9:  return 3;   // BLACK_ROOK -> WHITE_ROOK
            case 10: return 4;   // BLACK_QUEEN -> WHITE_QUEEN
            case 11: return 5;   // BLACK_KING -> WHITE_KING
            default: return pieceType;
        }
    }

    /// Changes the perspective of the given state.
    /// This rotates every bitboard 180° (using bb_utils::reverse),
    /// swaps the white and black piece arrays,
    /// rebuilds the typeAtSquare array by reversing its order and flipping each piece type
    void changePerspective(std::array<uint64_t, 12> &pieces, std::array<uint8_t, 64> &typeAtSquare, uint8_t &en_passant);
}

#endif // STATE_TRANSITION_HPP
