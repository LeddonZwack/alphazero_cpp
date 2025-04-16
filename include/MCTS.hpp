#ifndef MCTS_HPP
#define MCTS_HPP

#include "AlphaZeroTrainer.hpp"
#include "State.hpp"
#include "StateTransition.hpp"
#include "MoveGeneration.hpp"
#include "GameStatus.hpp"
#include <vector>
#include <cmath>
#include <unordered_map>

namespace MCTS {

    // The Node structure will live in an arena (a std::vector<Node>).
    // We use integer indices to refer to parent/children.
    struct Node {
        int action_taken;           // Action that led to this node (for root, can be -1)
        float prior;                // Prior probability from the policy network (set uniformly if not given)
        int visit_count;            // Number of visits
        float value_sum;            // Sum of simulation values
        // Use std::optional<Chess::State> state; in future to allow for lazy initialization
        Chess::State state;         // The game state at this node (by value)
        int parent;                 // Index of the parent node in the arena; -1 for root
        std::vector<int> children;  // Indices of child nodes in the arena

        Node(const Chess::State& state_, int action, float prior_, int parentIndex)
                : action_taken(action), prior(prior_), visit_count(0), value_sum(0.0f),
                  state(std::move(state_)), parent(parentIndex) { }

        // Returns the average value.
        inline float meanValue() const {
            return (visit_count == 0) ? 0.0f : value_sum / static_cast<float>(visit_count);
        }
    };

    // The MCTS class implements search over game states using a simple arena.
    // In Option A, the tree is built from scratch each search and discarded.
    class MCTS {
    public:
        // Constructor takes configuration (we assume TrainerArgs has at least num_searches and C).
        MCTS(const AlphaZeroTrainer::TrainerArgs& args);

        // Search: given a starting state and a reference to a repetition map (mapping state.zobrist_hash to count),
        // perform MCTS search and return a vector (of length action_size) of normalized visit counts (policy).
        std::array<float, AlphaZeroTrainer::action_size> search(const Chess::State& state,
                                  const std::unordered_map<uint64_t, uint8_t>& repetitionMap);

    private:
        int num_searches;
        double C;  // Exploration constant

        // The arena where nodes are stored during a single search.
        std::vector<Node> arena;

        // Helper functions:
        // Selection: starting at rootIdx, traverse children using UCB until a leaf is reached.
        int selectLeaf(int rootIdx);

        // Expansion: for node at index leafIdx, expand its children.
        // Returns true if expansion succeeded (i.e. node was non-terminal), false if terminal.
        void expandNode(int leafIdx, std::array<float, AlphaZeroTrainer::action_size> policy);

        // Backpropagation: update node statistics along the path from nodeIdx up to the root.
        void backpropagate(int nodeIdx, float value);

        // UCB formula: returns UCB score for a child given parent's visit count.
        inline float ucbScore(const Node& child, int parentVisits) const;
    };

} // namespace MCTS

#endif // MCTS_HPP