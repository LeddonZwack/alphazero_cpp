#ifndef MCTS_HPP
#define MCTS_HPP

#include <vector>
#include <cmath>
#include <unordered_map>
#include "AlphaZeroTrainer.hpp"
#include "AZTypes.hpp"
#include "State.hpp"
#include "StateTransition.hpp"
//#include "MoveGeneration.hpp"
#include "GameStatus.hpp"

class ModelInterface;  // forward

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
        bool clearMap;              // To see if we should clear map at that node

        Node(const Chess::State& state_, int action_, float prior_, int parentIndex_, bool clearMap_)
                : action_taken(action_), prior(prior_), visit_count(0), value_sum(0.0f),
                  state(state_), parent(parentIndex_), clearMap(clearMap_) { }


        // Returns the average value.
        inline float meanValue() const {
            return (visit_count == 0) ? 0.0f : value_sum / static_cast<float>(visit_count);

        }

        void print(int nodeIdx = -1) const {
                std::cout << "──── Node";
                if (nodeIdx != -1) std::cout << " #" << nodeIdx;
                std::cout << " ────\n";
                std::cout << "  action_taken : " << action_taken << "\n";
                std::cout << "  fromSqure : " << action_taken % 64 << " and moveType: " << action_taken / 64 << "\n";
                std::cout << "  prior         : " << prior << "\n";
                std::cout << "  visit_count   : " << visit_count << "\n";
                std::cout << "  value_sum     : " << value_sum << "\n";
                std::cout << "  mean_value    : " << meanValue() << "\n";
                std::cout << "  parent        : " << parent << "\n";
                std::cout << "  clearMap      : " << (clearMap ? "true" : "false") << "\n";
                std::cout << "  num_children  : " << children.size() << "\n";
                std::cout << "  state:\n";
                state.print();
                std::cout << "──────────────────────\n";
            }
    };

    // The MCTS class implements search over game states using a simple arena.
    // In Option A, the tree is built from scratch each search and discarded.
    class MCTS {
    public:
        // Constructor takes configuration (we assume TrainerArgs has at least num_searches and C).
        MCTS(const AlphaZeroTrainer::TrainerArgs& args, ModelInterface& modelInterface);

        // Search: given a starting state and a reference to a repetition map (mapping state.zobrist_hash to count),
        // perform MCTS search and return a vector (of length action_size) of normalized visit counts (policy).
        std::array<float, ACTION_SIZE> search(const Chess::State& state,
                                  const std::unordered_map<uint64_t, uint8_t>& repetitionMap);


    private:
        // Taking from TrainerArgs
        ModelInterface& modelIf_;
        int num_searches;
        double C;  // Exploration constant
        int historyLength;
        double dirichlet_epsilon;
        double dirichlet_alpha;

        // Arena for our class
        std::vector<Node> arena;

        // Helper functions:
        // Selection: starting at rootIdx, traverse children using UCB until a leaf is reached.
        int selectLeaf(int rootIdx, std::unordered_map<uint64_t, uint8_t>& repetitionMap);

        // Expansion: for node at index leafIdx, expand its children.
        // Returns true if expansion succeeded (i.e. node was non-terminal), false if terminal.
        void expandNode(int leafIdx, std::array<float, ACTION_SIZE> policy);

        // Backpropagation: update node statistics along the path from nodeIdx up to the root.
        void backpropagate(int nodeIdx, float value);

        // Build a vector of the previous historyLength states (including current)
        std::vector<Chess::State> getCurrentTStates(int nodeIdx);

        // Updates the state's repeated_state flag using its zobrist hash and the repetition map
        void updateRepetitionTracking(Chess::State& state, std::unordered_map<uint64_t, uint8_t>& repMap);

        // Debug
        void mctsDebugger(int leafIdx);

        // UCB formula: returns UCB score for a child given parent's visit count.
        inline float ucbScore(const Node& child, int parentVisits) const;
    };

} // namespace MCTS

#endif // MCTS_HPP