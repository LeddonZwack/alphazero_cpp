#include "AlphaZeroController.hpp"

AlphaZeroController::AlphaZeroController(const ControllerArgs& args) {
    // 1) Build your ResNet + optimizer
    ResNet net(args.gameConfig, args.numResBlocks, args.numHidden, args.device);
    auto opt = std::make_shared<torch::optim::Adam>(net->parameters(), args.learningRate);

    // 2) Wrap in ModelInterface
    modelInterface_ = std::make_unique<ModelInterface>(
            std::move(net),
            std::move(opt),
            args.gameConfig,
            args.trainerArgs.historyLength
    );

    // 3) Build trainer & player
    trainer_ = std::make_unique<AlphaZeroTrainer>(*modelInterface_, args.trainerArgs, args.gameConfig);
//    player_  = std::make_unique<GamePlayer>(*modelInterface_ /*, player‑specific args*/);
}

void AlphaZeroController::runTraining() {
    trainer_->learn();
}

void AlphaZeroController::runPlay() {
//    player_->playLoop();
}
