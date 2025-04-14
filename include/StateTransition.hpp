#ifndef STATE_TRANSITION_HPP
#define STATE_TRANSITION_HPP

#include "State.hpp"
#include "MoveMapping.hpp"
#include <cstdint>

namespace StateTransition {

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
}

#endif // STATE_TRANSITION_HPP
