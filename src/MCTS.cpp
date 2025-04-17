#include "MCTS.hpp"
#include "State.hpp"
#include "StateTransition.hpp"
#include "MoveGeneration.hpp"
#include "GameStatus.hpp"
#include "ModelInterface.hpp"
#include <limits>
#include <iostream>
#include <cassert>
#include <cmath>

namespace MCTS {

    // Constructor: extract parameters from args.
    // For simplicity, assume TrainerArgs has "num_searches" and "C" fields.
    MCTS::MCTS(const AlphaZeroTrainer::TrainerArgs& args, ModelInterface& modelInterface)
            : modelIf_(modelInterface), num_searches(args.num_searches), // or args.num_searches if defined
              C(args.C), historyLength(args.historyLength),
              dirichlet_epsilon(args.dirichlet_epsilon), dirichlet_alpha(args.dirichlet_alpha)
    {
        // Rough upper bound on max size of arena
        arena.reserve(218 * (1 + num_searches));
    }

    // UCB score: if a child has zero visits, we'll use its prior value as a bonus.
    inline float MCTS::ucbScore(const Node& child, int parentVisits) const {
        // We use a formulation similar to: UCB = Q + C * prior * sqrt(parentVisits) / (1 + child.visit_count)
        float Q = (1 - child.meanValue()) / 2; // What the guy in YT video did
        return Q + static_cast<float>(C)* child.prior * std::sqrt(static_cast<float>(parentVisits)) / (1.0f + child.visit_count);
    }

    // Selection: starting at rootIdx, traverse using ucbScore until reaching a leaf (no children).
    // Note, the map passed in is a copy of the repetition map given to us at beginning of search
    int MCTS::selectLeaf(int rootIdx, std::unordered_map<uint64_t, uint8_t>& copyRepMap) {
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

            // If we should clear the map, do so
            if (arena[currIdx].clearMap) {
                copyRepMap.clear();
            }

            // Handle updating repetitionMap and state's repeated state flag for states visited in selection
            auto& state = arena[currIdx].state;
            auto hash   = state.zobrist_hash;
            auto count  = ++copyRepMap[hash];  // increment and capture count
            StateTransition::updateRepeatedStateFlag(state, count);
        }
        return currIdx;
    }

    // Expansion: Given a node at index leafIdx, expand it by generating valid actions.
    // For simplicity, we use uniform priors for children.
    // Return true if the node was expanded (non-terminal); false if terminal.
    void MCTS::expandNode(int leafIdx, std::array<float, ACTION_SIZE> policy) {

        // Iterate over policy
        for (int action = 0; action < policy.size(); ++action) {
            float action_probability = policy[action];
            if (action_probability == 0) continue;

            // Check if we should clear map
            bool clearMap(false);

            // Add child to arena
            arena.emplace_back(StateTransition::getCopyNextState(arena[leafIdx].state, action, clearMap), action, action_probability, leafIdx, clearMap);

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

    // Build a vector of the previous historyLength states (including current)
    std::vector<Chess::State> MCTS::getCurrentTStates(int nodeIdx) {
        std::vector<Chess::State> result;

        int currIdx = nodeIdx;
        while (currIdx != -1 && result.size() < historyLength) {
            result.push_back(arena[currIdx].state);
            currIdx = arena[currIdx].parent;
        }

        // If fewer than historyLength_ were found, pad with copies of the oldest state
        while (result.size() < historyLength && !result.empty()) {
            result.push_back(result.back());
        }

        // Reverse to get states from oldest to newest (chronological order)
        std::reverse(result.begin(), result.end());

        return result;
    }

    // Main search function.
    // Takes in a root state and a repetition map (to update repeated state counts).
    // Returns a vector of normalized visit counts for each possible action (of length equal to the action size).
    std::array<float, ACTION_SIZE> MCTS::search(const Chess::State& rootState,
                                    const std::unordered_map<uint64_t, uint8_t>& repetitionMap) {
        // Clear the arena.
        arena.clear();

        // Create the root node; parent index = -1, action_taken = -1.
        Node root(rootState, -1, 1.0f, -1, false);
        root.visit_count = 1; // Set initial visit count.
        arena.push_back(root);

        // Get initial states
        std::vector<Chess::State> rootStates;
        rootStates.reserve(historyLength);

        for (int t = 0; t < historyLength; ++t) {
            rootStates.emplace_back(arena[0].state);
        }

        // Get policy for root
        auto [rawPolicyRoot, _] = modelIf_.evaluateWithNetwork(rootStates);

        // Add DirichletNoise to root only
        auto noiseAddedPolicyRoot = modelIf_.addDirichletNoise(rawPolicyRoot, dirichlet_epsilon, dirichlet_alpha);

        // Get root's valid moves
        std::array<bool, ACTION_SIZE> validMovesRoot = MoveGeneration::getValidMoves(arena[0].state);

        // Masked policy for root
        auto policyRoot = modelIf_.maskAndNormalizePolicy(rawPolicyRoot, validMovesRoot);

        // Expand root
        expandNode(0, policyRoot);

        // Perform MCTS iterations.
        for (int iter = 0; iter < num_searches; ++iter) {

            // Create a copy of repetition map for this search through tree
            auto copyRepMap = repetitionMap;

            // Selection: starting at root, select a leaf.
            int leafIdx = selectLeaf(0, copyRepMap);

            // Calculate valid moves here
            std::array<bool, ACTION_SIZE> validMovesLeaf = MoveGeneration::getValidMoves(arena[leafIdx].state);

            // Evaluate the state
            auto [intVal, isTerminal] = GameStatus::evaluateState(arena[leafIdx].state, &validMovesLeaf);
            auto value = static_cast<float>(-intVal);

            if (!isTerminal) {
                // Get current T states from leaf (including itself)
                auto currentStates = getCurrentTStates(leafIdx);

                // Get policy for root
                auto [rawPolicyLeaf, modelValue] = modelIf_.evaluateWithNetwork(currentStates);

                // Masked policy for root
                auto policyLeaf = modelIf_.maskAndNormalizePolicy(rawPolicyLeaf, validMovesLeaf);

                // Set value to modelValue
                value = modelValue;

                // Expand node
                expandNode(leafIdx, policyLeaf);
            }

            // Backpropagation: update the tree along the selected path.
            backpropagate(leafIdx, value);
        }

        // Create and return action probabilities
        std::array<float, ACTION_SIZE> action_probs{};
        float sum = 0.0f;

        for (int childIdx : arena[0].children) {
            const auto& child = arena[childIdx];
            auto visits = static_cast<float>(child.visit_count);
            action_probs[child.action_taken] = visits;
            sum += visits;
        }

        // Normalize if sum > 0, should always occur
        if (sum > 0.0f) {
            for (float& p : action_probs) {
                p /= sum;
            }
        }

        return action_probs;
    }

} // namespace MCTS