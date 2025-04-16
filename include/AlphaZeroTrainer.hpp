#ifndef ALPHA_ZERO_TRAINER_HPP
#define ALPHA_ZERO_TRAINER_HPP

#include <torch/torch.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "State.hpp"
#include "StateEncoder.hpp"

/// AlphaZeroTrainer is responsible for self-play and training.
/// Note: MCTS is instantiated locally within selfPlay.
class AlphaZeroTrainer {
public:
    // Fixed constants
    static constexpr int row_count = 8;
    static constexpr int col_count = 8;
    static constexpr int action_size = 4672;

    // TrainerArgs holds configuration parameters.
    struct TrainerArgs {
        int num_iterations;
        int num_selfPlay_iterations;
        int num_epochs;
        int batch_size;
        double temperature;
        double dirichlet_epsilon;
        double dirichlet_alpha;
        double C;
        int historyLength;
    };

    // A training example: encoded state, target policy, and target value.
    struct TrainingExample {
        std::vector<float> encodedState;
        std::array<float, action_size> policyTarget;
        int valueTarget;
    };

    // Local structure to record self-play history.
    struct SelfPlayRecord {
        std::vector<Chess::State> states;
        std::array<float, action_size> actionProbs;
        int player; // +1, -1
    };

    // Constructor: receives a LibTorch model, optimizer, and training arguments.
    AlphaZeroTrainer(std::shared_ptr<torch::nn::Module> model,
                     std::shared_ptr<torch::optim::Optimizer> optimizer,
                     const TrainerArgs& args);

    // Self-play method: returns a vector of training examples.
    std::vector<TrainingExample> selfPlay();

    // Training method (stub).
    void train(const std::vector<TrainingExample>& memory);

    // Learning loop (stub).
    void learn();

    // Get encoded snapshot and flags helper
    std::pair<std::vector<Chess::HistorySnapshot>, Chess::StateFlags> getEncodedSnapshotAndFlags(const std::vector<Chess::State>& states);

    // Public members.
    std::shared_ptr<torch::nn::Module> model;
    std::shared_ptr<torch::optim::Optimizer> optimizer;
    TrainerArgs args;
};

#endif // ALPHA_ZERO_TRAINER_HPP
