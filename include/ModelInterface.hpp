// include/ModelInterface.hpp
#ifndef MODEL_INTERFACE_HPP
#define MODEL_INTERFACE_HPP

#include <torch/torch.h>
#include <vector>
#include <array>
#include <random>
#include "Network.hpp"            // ResNet, GameConfig
#include "StateEncoder.hpp"       // StateEncoder::encodeState
#include "MoveGeneration.hpp"     // MoveGeneration::getValidMoves
#include "AZTypes.hpp"         // for ACTION_SIZE & TrainingExample
#include "State.hpp"              // Chess::HistorySnapshot, Chess::StateFlags

class ModelInterface;  // forward, so no circular include

class ModelInterface {
public:
    using PolicyArray = std::array<float, ACTION_SIZE>;

    // -- Ctor takes your ResNet handle by value (ModuleHolder<ResNetImpl>) --
    ModelInterface(ResNet model,
                   std::shared_ptr<torch::optim::Optimizer> optimizer,
                   const GameConfig& config,
                   int historyLength);

    // Get encoded snapshot and flags helper
    static std::pair<std::vector<Chess::HistorySnapshot>, Chess::StateFlags>
                        getEncodedSnapshotAndFlags(const std::vector<Chess::State>& states);

    // Encode + forward the network → (policy_probs, value)
    std::pair<PolicyArray, float>
    evaluateWithNetwork(const std::vector<Chess::State>& states);

    // Mask illegal moves & renormalize
    PolicyArray
    maskAndNormalizePolicy(const PolicyArray& rawPolicy,
                           const std::array<bool, ACTION_SIZE>& validMoves);

    // One gradient step on a batch of examples
    void trainBatch(const std::vector<TrainingExample>& batch);

    // Dirichlet noise for MCTS root noise injection
    // Returns: (1-ε)*policy + ε*Dir(alpha)
    PolicyArray addDirichletNoise(const PolicyArray& policy,
                                  double dirichlet_epsilon,
                                  double dirichlet_alpha);

private:
    ResNet                                 model_;          // module holder
    std::shared_ptr<torch::optim::Optimizer> optimizer_;
    GameConfig                             config_;
    int                                    historyLength_;
};

#endif // MODEL_INTERFACE_HPP
