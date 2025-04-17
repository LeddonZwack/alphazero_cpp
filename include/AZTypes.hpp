// include/AZTypes.hpp
#ifndef AZ_TYPES_HPP
#define AZ_TYPES_HPP

#include <vector>
#include <array>

// your fixed action space size
static constexpr int ACTION_SIZE = 4672;

/// A single training example: (encodedState, targetPolicy, targetValue)
struct TrainingExample {
    std::vector<float>             encodedState;
    std::array<float, ACTION_SIZE> policyTarget;
    int                            valueTarget;
};

// A struct to hold game configuration parameters.
struct GameConfig {
    int T;             // History length.
    int row_count;     // Number of rows in the board.
    int column_count;  // Number of columns in the board.
    int action_size;   // Total number of possible actions.
};

#endif // AZ_TYPES_HPP
