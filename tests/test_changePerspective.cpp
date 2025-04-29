//
// Created by Leddon Zwack on 4/19/25.
//

# include "test_changePerspective.hpp"
# include "State.hpp"
# include "StateTransition.hpp"
# include "bitboard/piece_type.hpp"
# include "MoveGeneration.hpp"

#include <iostream>
#include <string>

namespace tt {

    void runTests() {

        std::cout << "Testing Black King Captured" << std::endl;

        Chess::State state;

        std::cout << "Initial board" << std::endl;
        state.validateAndPrintBoard();

        StateTransition::getNextState(state, 13);

        state.validateAndPrintBoard();

        StateTransition::getNextState(state, 11);

        state.validateAndPrintBoard();

        StateTransition::getNextState(state, 3268);

        state.validateAndPrintBoard();

        // Check valid moves for white
        auto [validMoves, debug] = MoveGeneration::getValidMoves(state);

    }
}