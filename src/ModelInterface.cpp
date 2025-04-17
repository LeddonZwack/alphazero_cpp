// src/ModelInterface.cpp
#include "ModelInterface.hpp"
#include <cassert>

ModelInterface::ModelInterface(ResNet model,
                               std::shared_ptr<torch::optim::Optimizer> optimizer,
                               const GameConfig& config,
                               int historyLength)
        : model_(std::move(model))
        , optimizer_(std::move(optimizer))
        , config_(config)
        , historyLength_(historyLength)
{
    // Put the module in training mode
    model_->train(true);
}

std::pair<std::vector<Chess::HistorySnapshot>, Chess::StateFlags>
        ModelInterface::getEncodedSnapshotAndFlags(const std::vector<Chess::State>& states) {

    std::vector<Chess::HistorySnapshot> history;
    for (const Chess::State& state : states) {
        history.push_back(state.getHistorySnapshot());
    }
    return {history, states[states.size()-1].flags};
}

std::pair<ModelInterface::PolicyArray, float>
        ModelInterface::evaluateWithNetwork(const std::vector<Chess::State>& states)
{
    // Get encoded history and flags from helper
    auto [history, flags] = getEncodedSnapshotAndFlags(states);

    // 1) encode → flat vector
    auto flat = StateEncoder::encodeState(history, flags, historyLength_);

    // 2) wrap in [1, C, H, W] tensor
    int C = (14 * historyLength_) + 7;
    auto input = torch::from_blob(
            flat.data(),
            {1, C, config_.row_count, config_.column_count},
            torch::kFloat)
            .clone();

    // 3) forward: ResNetImpl::forward returns pair<logits, value>
    auto [logits, value_t] = model_->forward(input);

    // 4) softmax → policy
    auto probs = torch::softmax(logits, /*dim=*/1);
    PolicyArray policy{};
    for (int i = 0; i < policy.size(); ++i) {
        policy[i] = probs[0][i].item<float>();
    }

    float value = value_t.item<float>();
    return {policy, value};
}

ModelInterface::PolicyArray ModelInterface::maskAndNormalizePolicy(const PolicyArray& rawPolicy,
                                       const std::array<bool, ACTION_SIZE>& validMoves)
{
    PolicyArray out{};
    float sum = 0.0f;
    for (int i = 0; i < (int)out.size(); ++i) {
        if (validMoves[i]) {
            out[i] = rawPolicy[i];
            sum += rawPolicy[i];
        }
    }
    if (sum > 0.0f) {
        for (auto& x : out) x /= sum;
    } else {
        // fallback to uniform
        int cnt = 0;
        for (bool v : validMoves) cnt += (v ? 1 : 0);
        float u = cnt ? (1.0f / cnt) : 0.0f;
        for (int i = 0; i < (int)out.size(); ++i)
            if (validMoves[i]) out[i] = u;
    }
    return out;
}

void ModelInterface::trainBatch(const std::vector<TrainingExample>& batch)
{
    // ensure training mode
    model_->train(true);

    // build tensors
    std::vector<torch::Tensor> states, policies, values;
    states.reserve(batch.size());
    policies.reserve(batch.size());
    values.reserve(batch.size());

    int C = (14 * historyLength_) + 7;
    auto H = config_.row_count, W = config_.column_count;

    for (auto const& ex : batch) {
        // state
        auto s = torch::from_blob(
                const_cast<float*>(ex.encodedState.data()),
                {1, C, H, W},
                torch::kFloat)
                .clone();
        states.push_back(std::move(s));

        // policy target
        auto p = torch::from_blob(
                const_cast<float*>(ex.policyTarget.data()),
                {ACTION_SIZE},
                torch::kFloat)
                .clone();
        policies.push_back(std::move(p));

        // value target
        values.push_back(torch::tensor(ex.valueTarget, torch::kFloat));
    }

    auto X = torch::cat(states, 0);
    auto P = torch::stack(policies, 0);
    auto V = torch::stack(values,   0);

    // forward
    auto [logits, preds] = model_->forward(X);

    // losses
    auto logP       = torch::log_softmax(logits, /*dim=*/1);
    auto policyLoss = - (P * logP).sum(1).mean();
    auto valueLoss  = torch::mse_loss(preds.view({-1}), V);

    auto loss = policyLoss + valueLoss;

    // backward & step
    optimizer_->zero_grad();
    loss.backward();
    optimizer_->step();
}

ModelInterface::PolicyArray
ModelInterface::addDirichletNoise(const PolicyArray& policy,
                                  double dirichlet_epsilon,
                                  double dirichlet_alpha)
{
    // 1) Sample α‑parameterized Gamma variables
    std::mt19937_64                    rng{ std::random_device{}() };
    std::gamma_distribution<double>    gammaDist(dirichlet_alpha, 1.0);

    PolicyArray noise{};
    double sum = 0.0;
    for (int i = 0; i < (int)noise.size(); ++i) {
        noise[i] = static_cast<float>(gammaDist(rng));
        sum += noise[i];
    }
    // 2) Normalize to get Dirichlet draw
    if (sum > 0.0) {
        for (auto &x : noise) x = static_cast<float>(x / sum);
    }

    // 3) Mix original policy with noise
    PolicyArray out{};
    for (int i = 0; i < (int)out.size(); ++i) {
        out[i] = static_cast<float>(
                (1.0 - dirichlet_epsilon) * policy[i]
                + dirichlet_epsilon       * noise[i]
        );
    }
    return out;
}