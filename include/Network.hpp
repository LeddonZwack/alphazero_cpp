#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <torch/torch.h>

// A struct to hold game configuration parameters.
struct GameConfig {
    int T;             // History length.
    int row_count;     // Number of rows in the board.
    int column_count;  // Number of columns in the board.
    int action_size;   // Total number of possible actions.
};

// ----------------------- ResBlock ----------------------------

struct ResBlockImpl : torch::nn::Module {
    // Layers for the residual block.
    torch::nn::Conv2d conv1{ nullptr };
    torch::nn::BatchNorm2d bn1{ nullptr };
    torch::nn::Conv2d conv2{ nullptr };
    torch::nn::BatchNorm2d bn2{ nullptr };

    ResBlockImpl(int num_hidden);
    torch::Tensor forward(torch::Tensor x);
};
TORCH_MODULE(ResBlock);

// ----------------------- ResNet ------------------------------

struct ResNetImpl : torch::nn::Module {
    GameConfig config;
    int num_resBlocks;
    int num_hidden;

    // Shared layers.
    torch::nn::Sequential startBlock{ nullptr };
    torch::nn::ModuleList backBone;
    torch::nn::Sequential policyHead{ nullptr };
    torch::nn::Sequential valueHead{ nullptr };

    ResNetImpl(const GameConfig& config, int num_resBlocks, int num_hidden, torch::Device device);
    // Forward function returns a pair: policy logits and value.
    std::pair<torch::Tensor, torch::Tensor> forward(torch::Tensor x);
};
TORCH_MODULE(ResNet);

#endif // NETWORK_HPP
