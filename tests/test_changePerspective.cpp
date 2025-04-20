//
// Created by Leddon Zwack on 4/19/25.
//

# include "test_changePerspective.hpp"
# include "State.hpp"
# include "StateTransition.hpp"
# include "bitboard/piece_type.hpp"

#include <iostream>
#include <string>

namespace tt {

    void runTests() {

        std::cout << "Testing State Transition and Change Perspective" << std::endl;

        Chess::State state1;

        std::cout << "Initial board" << std::endl;
        state1.validateAndPrintBoard();

        int action = 8;

        StateTransition::getNextState(state1, action);

        std::cout << "Post getNextState" << std::endl;
        state1.validateAndPrintBoard();

        Chess::State state2;

        // Bitboards
        uint64_t original = state2.pieces[0];
        uint64_t moved = (original & ~(1ULL << 8)) | (1ULL << 16);
        state2.pieces[0] = moved;

        // TypeAtSquare
        state2.typeAtSquare[8] = bb::NO_PIECE;
        state2.typeAtSquare[16] = bb::WHITE_PAWN;

        std::cout << "Original with action" << std::endl;
        state2.validateAndPrintBoard();

        StateTransition::changePerspective(state2.pieces, state2.typeAtSquare, state2.flags.en_passant);

        std::cout << "Flipped with action" << std::endl;
        state2.validateAndPrintBoard();

    }
}