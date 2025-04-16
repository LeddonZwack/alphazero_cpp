#include "MCTS.hpp"
#include "State.hpp"
#include "StateTransition.hpp"
#include "MoveGeneration.hpp"
#include "GameStatus.hpp"
#include <limits>
#include <iostream>
#include <cassert>
#include <cmath>

namespace MCTS {

    /// ------------------ Network Helper Functions ----------------------///
    // Assume that AlphaZeroTrainer::action_size, GameConfig, and the HistorySnapshot/StateFlags types
    // are defined as in your provided code.

    // This helper evaluates a given Chess::State using the network model.
    // It converts the state via your encoder into a torch::Tensor input and forwards it through the model.
    std::pair<std::array<float, AlphaZeroTrainer::action_size>, float>
    evaluateWithNetwork(const Chess::State &state,
                        ResNet &model,
                        const GameConfig &config,
                        int historyLength) {
        // Assume the Chess::State provides these getters.
        const std::vector<Chess::HistorySnapshot>& history = state.getHistory();
        const Chess::StateFlags& flags = state.getFlags();

        // Encode state using your helper function.
        std::vector<float> encoded = StateEncoder::encodeState(history, flags, historyLength);

        // Determine tensor dimensions:
        // channels = (14 * historyLength) + 7 (must agree with your network constructor)
        int channels = (14 * historyLength) + 7;
        int height = config.row_count;
        int width = config.column_count;

        // Create a tensor from the flat vector.
        // Note: .clone() is used to ensure the tensor owns its data.
        torch::Tensor inputTensor = torch::from_blob(encoded.data(), {1, channels, height, width}, torch::kFloat).clone();

        // Forward pass through the model.
        auto [policyLogits, valueTensor] = model->forward(inputTensor);

        // Convert logits to probabilities via softmax over the action dimension.
        torch::Tensor policyTensor = torch::softmax(policyLogits, /*dim=*/1);

        // Copy probabilities to a std::array.
        std::array<float, AlphaZeroTrainer::action_size> policy{};
        for (int i = 0; i < AlphaZeroTrainer::action_size; i++) {
            policy[i] = policyTensor[0][i].item<float>();
        }

        float value = valueTensor.item<float>();
        return {policy, value};
    }

    // This helper masks the network’s raw policy probabilities with the valid moves mask
    // and then normalizes the distribution.
    std::array<float, AlphaZeroTrainer::action_size>
    maskAndNormalizePolicy(const std::array<float, AlphaZeroTrainer::action_size> &rawPolicy,
                           const std::array<bool, AlphaZeroTrainer::action_size> &validMoves) {
        std::array<float, AlphaZeroTrainer::action_size> maskedPolicy{};
        float sum = 0.0f;

        // Apply the valid moves mask.
        for (int i = 0; i < AlphaZeroTrainer::action_size; i++) {
            if (validMoves[i]) {
                maskedPolicy[i] = rawPolicy[i];
                sum += rawPolicy[i];
            } else {
                maskedPolicy[i] = 0.0f;
            }
        }

        // Normalize the probabilities if the sum is positive.
        if (sum > 0.0f) {
            for (int i = 0; i < AlphaZeroTrainer::action_size; i++) {
                maskedPolicy[i] /= sum;
            }
        } else {
            // In the unlikely case that all probabilities are zero,
            // assign uniform probabilities to valid moves.
            int count = 0;
            for (bool valid : validMoves) {
                if (valid)
                    count++;
            }
            if (count > 0) {
                float uniformProb = 1.0f / count;
                for (int i = 0; i < AlphaZeroTrainer::action_size; i++) {
                    if (validMoves[i])
                        maskedPolicy[i] = uniformProb;
                }
            }
        }

        return maskedPolicy;
    }

    /// ------------------ MCTS Functions ----------------------///

    // Constructor: extract parameters from args.
    // For simplicity, assume TrainerArgs has "num_searches" and "C" fields.
    MCTS::MCTS(const AlphaZeroTrainer::TrainerArgs& args)
            : num_searches(args.num_selfPlay_iterations), // or args.num_searches if defined
              C(args.C)
    {
        // Nothing else to do.
    }

    // UCB score: if a child has zero visits, we'll use its prior value as a bonus.
    inline float MCTS::ucbScore(const Node& child, int parentVisits) const {
        // We use a formulation similar to: UCB = Q + C * prior * sqrt(parentVisits) / (1 + child.visit_count)
        float Q = (1 - child.meanValue()) / 2;
        return Q + C * child.prior * std::sqrt(static_cast<float>(parentVisits)) / (1.0f + child.visit_count);
    }

    // Selection: starting at rootIdx, traverse using ucbScore until reaching a leaf (no children).
    int MCTS::selectLeaf(int rootIdx) {
        int currIdx = rootIdx;
        while (!arena[currIdx].children.empty()) {
            int bestChildIdx = -1;
            float bestScore = -std::numeric_limits<float>::infinity();
            int parentVisits = arena[currIdx].visit_count;
            for (int childIdx : arena[currIdx].children) {
                float score = ucbScore(arena[childIdx], parentVisits);
                if (score > bestScore) {
                    bestScore = score;
                    bestChildIdx = childIdx;
                }
            }
            if (bestChildIdx == -1)
                break;
            currIdx = bestChildIdx;
        }
        return currIdx;
    }

    // Expansion: Given a leaf node at index leafIdx, expand it by generating valid actions.
    // For simplicity, we use uniform priors for children.
    // Return true if the node was expanded (non-terminal); false if terminal.
    void MCTS::expandNode(int leafIdx, std::array<float, AlphaZeroTrainer::action_size> policy) {

        // Iterate over policy
        for (int action = 0; action < policy.size(); ++action) {
            float action_probability = policy[action];
            if (action_probability == 0) continue;

            // Add child to arena
            arena.emplace_back(StateTransition::getCopyNextState(arena[leafIdx].state, action), action, action_probability, leafIdx);

            // Add child index to children
            arena[leafIdx].children.emplace_back(static_cast<int>(arena.size()) - 1);
        }
    }

    // Backpropagation: from nodeIdx, update ancestors with simulation value.
    void MCTS::backpropagate(int nodeIdx, float value) {
        int currIdx = nodeIdx;
        while (currIdx != -1) {
            arena[currIdx].visit_count += 1;
            arena[currIdx].value_sum += value;
            // Flip the value for the opponent.
            value = -value;
            currIdx = arena[currIdx].parent;
        }
    }

    // Main search function.
    // Takes in a root state and a repetition map (to update repeated state counts).
    // Returns a vector of normalized visit counts for each possible action (of length equal to the action size).
    std::array<float, AlphaZeroTrainer::action_size> MCTS::search(const Chess::State& rootState,
                                    const std::unordered_map<uint64_t, uint8_t>& repetitionMap) {
        // Clear the arena.
        arena.clear();
        // Rough upper bound on max size of arena
        arena.reserve(218 * (1 + num_searches));

        // Create the root node; parent index = -1, action_taken = -1.
        Node root(rootState, -1, 1.0f, -1);
        root.visit_count = 1; // Set initial visit count.
        arena.push_back(root);

        // TODO: Get policy and value (don't care about value tho) output from model (helper function)
        std::array<float, AlphaZeroTrainer::action_size> policy {}; // Dummy

        // Get root's valid moves
        std::array<bool, AlphaZeroTrainer::action_size> valid_moves_root = MoveGeneration::getValidMoves(arena[0].state);

        // TODO: Mask policy with valid moves and normalize (make helper function)

        // Expand root
        expandNode(0, policy);

        // Perform MCTS iterations.
        for (int iter = 0; iter < num_searches; ++iter) {
            // Selection: starting at root, select a leaf.
            int leafIdx = selectLeaf(0);

            // Calculate valid moves here
            std::array<bool, AlphaZeroTrainer::action_size> valid_moves = MoveGeneration::getValidMoves(arena[leafIdx].state);

            // Evaluate the state
            auto [value, isTerminal] = GameStatus::evaluateState(arena[leafIdx].state);
            value = static_cast<float>(-value);

            if (!isTerminal) {
                // TODO: Get policy and value (value overrides above) output from model (helper function)

                std::array<float, AlphaZeroTrainer::action_size> policy {}; // Dummy

                // TODO: Mask policy with valid moves and normalize (make helper function)

                // Expand node
                expandNode(leafIdx, policy);
            }

            // Backpropagation: update the tree along the selected path.
            backpropagate(leafIdx, value);
        }

        std::array<float, AlphaZeroTrainer::action_size> action_probabilities {};
        for (auto child_idx: arena[0].children) {
            action_probabilities[arena[child_idx].action_taken] = static_cast<float>(arena[child_idx].visit_count);
        }
        // TODO: Normalize action_probabilities
        return action_probabilities;
    }

} // namespace MCTS
    // TODO: Logic for repeated_state map and getting the input for the model
