#ifndef ALPHA_ZERO_TRAINER_HPP
#define ALPHA_ZERO_TRAINER_HPP

#include <vector>
#include <array>
#include "AZTypes.hpp"   // for TrainingExample & ACTION_SIZE
#include "State.hpp"
//#include "StateEncoder.hpp"

class ModelInterface;    // just a forward declaration

class AlphaZeroTrainer {
public:
    struct TrainerArgs {
        int    num_iterations;
        int    num_selfPlay_iterations;
        int    num_searches;
        int    num_epochs;
        int    batch_size;
        double temperature;
        double dirichlet_epsilon;
        double dirichlet_alpha;
        double C;
        int    historyLength;
    };

    AlphaZeroTrainer(ModelInterface& modelInterface,
                     TrainerArgs trainerArgs, GameConfig gameConfig);

    /// runs one episode of self-play, returns training examples
    std::vector<TrainingExample> selfPlay();

    /// given a batch, calls ModelInterface::trainBatch
    void train(const std::vector<TrainingExample>& memory);

    void logCheckpoint(int iteration);

    /// full loop: selfPlay() + train() repeated num_iterations
    void learn();


private:
    ModelInterface& modelIf_;
    TrainerArgs    trainerArgs_;
    GameConfig     gameConfig_;

};

#endif // ALPHA_ZERO_TRAINER_HPP
