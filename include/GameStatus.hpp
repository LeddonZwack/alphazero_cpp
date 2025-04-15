#ifndef GAME_STATUS_HPP
#define GAME_STATUS_HPP

#include "State.hpp"
#include <array>
#include <utility>

// The GameStatus module is responsible for determining if a state is terminal and,
// if so, returning the appropriate outcome value.
// The evaluateState function returns a pair: {value, terminated} where:
//   - value: 1 if checkmate (White wins), 0 otherwise (draw, stalemate, insufficient material),
//   - terminated: true if the game is over.
namespace GameStatus {

    // Evaluate the given state.
    // If valid_moves is not provided, it is generated using MoveGeneration::getValidMoves(state).
    std::pair<int, bool> evaluateState(const Chess::State &state,
                                       const std::array<bool, 4672> *valid_moves_ptr = nullptr);

}

#endif // GAME_STATUS_HPP
