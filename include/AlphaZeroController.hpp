#ifndef ALPHA_ZERO_CONTROLLER_HPP
#define ALPHA_ZERO_CONTROLLER_HPP

#include <memory>
#include <torch/torch.h>
#include "Network.hpp"            // for GameConfig
#include "ModelInterface.hpp"
#include "AlphaZeroTrainer.hpp"
//#include "GamePlayer.hpp"         // your human‑vs‑AI loop

// Bundles all config needed to build model, trainer, player, etc.
struct ControllerArgs {
    GameConfig                       gameConfig;
    int                              numResBlocks;
    int                              numHidden;
    torch::Device                    device;
    double                           learningRate;
    AlphaZeroTrainer::TrainerArgs    trainerArgs;
    // Add playerArgs here later
};

class AlphaZeroController {
public:
    explicit AlphaZeroController(const ControllerArgs& args);

    /// Kicks off the full train‑selfplay‑train loop
    void runTraining();

    /// Launches a human vs. AI play loop
    void runPlay();

private:
    std::unique_ptr<ModelInterface>    modelInterface_;
    std::unique_ptr<AlphaZeroTrainer>  trainer_;
//    std::unique_ptr<GamePlayer>        player_;
};

#endif // ALPHA_ZERO_CONTROLLER_HPP
