#include "Network.hpp"

// -------------------- ResBlock Implementation --------------------
ResBlockImpl::ResBlockImpl(int num_hidden) {
    // Initialize the two convolution layers with kernel size 3 and padding 1.
    conv1 = register_module("conv1", torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, num_hidden, /*kernel_size=*/3).padding(1)));
    bn1   = register_module("bn1", torch::nn::BatchNorm2d(num_hidden));
    conv2 = register_module("conv2", torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, num_hidden, /*kernel_size=*/3).padding(1)));
    bn2   = register_module("bn2", torch::nn::BatchNorm2d(num_hidden));
}

torch::Tensor ResBlockImpl::forward(torch::Tensor x) {
    auto residual = x.clone();
    x = torch::relu(bn1(conv1(x)));
    x = bn2(conv2(x));
    x += residual;
    x = torch::relu(x);
    return x;
}

// -------------------- ResNet Implementation --------------------
ResNetImpl::ResNetImpl(const GameConfig& config, int num_resBlocks, int num_hidden, torch::Device device)
        : config(config), num_resBlocks(num_resBlocks), num_hidden(num_hidden) {

    // Input channels: (14 * T) + 7.
    int input_channels = (14 * config.T) + 7;

    // startBlock: Conv2d -> BatchNorm2d -> ReLU.
    startBlock = register_module("startBlock", torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(input_channels, num_hidden, /*kernel_size=*/3).padding(1)),
            torch::nn::BatchNorm2d(num_hidden),
            torch::nn::ReLU()
    ));

    // backBone: a ModuleList of residual blocks.
    backBone = register_module("backBone", torch::nn::ModuleList());
    for (int i = 0; i < num_resBlocks; ++i) {
        backBone->push_back(ResBlock(num_hidden));
    }

    // Policy head:
    // Conv2d(num_hidden, 128, kernel_size=3, padding=1) -> BatchNorm2d -> ReLU -> Flatten -> Linear.
    int flatten_size_policy = 128 * config.row_count * config.column_count;
    policyHead = register_module("policyHead", torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, 128, /*kernel_size=*/3).padding(1)),
            torch::nn::BatchNorm2d(128),
            torch::nn::ReLU(),
            torch::nn::Flatten(),
            torch::nn::Linear(flatten_size_policy, config.action_size)
    ));

    // Value head:
    // Conv2d(num_hidden, 64, kernel_size=3, padding=1) -> BatchNorm2d -> ReLU -> Flatten -> Linear -> Tanh.
    int flatten_size_value = 64 * config.row_count * config.column_count;
    valueHead = register_module("valueHead", torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, 64, /*kernel_size=*/3).padding(1)),
            torch::nn::BatchNorm2d(64),
            torch::nn::ReLU(),
            torch::nn::Flatten(),
            torch::nn::Linear(flatten_size_value, 1),
            torch::nn::Tanh()
    ));

    // Move the entire model to the specified device.
    to(device);
}

std::pair<torch::Tensor, torch::Tensor> ResNetImpl::forward(torch::Tensor x) {
    x = startBlock->forward(x);
    // Pass through each residual block.
    for (auto& block : *backBone) {
        x = block->as<ResBlock>()->forward(x);
    }
    // Get policy and value outputs.
    auto policy = policyHead->forward(x);
    auto value = valueHead->forward(x);
    return {policy, value};
}
