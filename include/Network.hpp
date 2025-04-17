#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "AZTypes.hpp"   // for GameConfict
#include <torch/torch.h>



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
