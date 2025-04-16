#include "AlphaZeroTrainer.hpp"
#include "MCTS.hpp"            // Our MCTS module.
#include "StateTransition.hpp" // Provides getCopyNextState, etc.
#include "GameStatus.hpp"      // Provides evaluateState.
//#include "StateEncoder.hpp"
#include <torch/torch.h>
#include <queue>
#include <iostream>
#include <random>
#include <algorithm>

// -------------------------- Helper: Random Sampling ------------------------
// Helper: sample an action index from a probability distribution.
static int sampleAction(const std::vector<float>& probs) {
    std::random_device rd;
    std::mt19937 gen(rd());
    float total = 0.0f;
    for (float p : probs) {
        total += p;
    }
    std::uniform_real_distribution<> dis(0.0, total);
    float r = dis(gen);
    float cumulative = 0.0f;
    for (size_t i = 0; i < probs.size(); ++i) {
        cumulative += probs[i];
        if (r <= cumulative)
            return static_cast<int>(i);
    }
    return static_cast<int>(probs.size() - 1);
}

// ---------------------- AlphaZeroTrainer Implementation ---------------------
AlphaZeroTrainer::AlphaZeroTrainer(std::shared_ptr<torch::nn::Module> model,
                                   std::shared_ptr<torch::optim::Optimizer> optimizer,
                                   const TrainerArgs& args)
        : model(model), optimizer(optimizer), args(args)
{
    // Constructor body. Additional initialization as required.
}

std::vector<AlphaZeroTrainer::TrainingExample> AlphaZeroTrainer::selfPlay() {
    // Starting player.
    int player = 1;
    // Initialize the state using the default constructor (starting position).
    Chess::State state;
    // Instantiate a local MCTS searcher for this move.
    MCTS::MCTS mctsSearcher(args); // Construct with args or configuration as needed.

    // Vector to hold memory
    std::vector<SelfPlayRecord> memory;
    // Create an external repetition map: maps a state's Zobrist hash to the count.
    std::unordered_map<uint64_t, uint8_t> repetitionMap;
    // Create a queue to hold previous T states
    std::queue<Chess::State> currentTStates;

    // Populate queue with initial state
    for (int i = 0; i < args.historyLength; ++i) {
        currentTStates.push(state);
    }

    // Insert root state's hash.
    repetitionMap[state.zobrist_hash] = 1;

    // Each self-play episode runs until game termination.
    while (true) {
        // Call mctsSearcher.search(state, repetitionMap)
        // Assume that mctsSearcher.search accepts the current state and a reference to the repetition map.
        std::array<float, action_size> actionProbs = mctsSearcher.search(state, repetitionMap);

        // Create a record and fill it from the queue
        AlphaZeroTrainer::SelfPlayRecord record;

        /// Handle filling memory with the current entry
        // Copy all states from currentTStates (FIFO) into the record
        std::queue<Chess::State> copyQueue = currentTStates;  // work on a copy so we don't destroy original
        while (!copyQueue.empty()) {
            record.states.push_back(copyQueue.front());
            copyQueue.pop();
        }
        // Add rest of the data
        record.actionProbs = actionProbs;
        record.player = player;
        // Push the full record into memory
        memory.push_back(record);

        // Adjust probabilities using temperature.
        std::vector<float> temperedProbs(actionProbs.size());
        float sum = 0.0f;
        for (size_t i = 0; i < actionProbs.size(); ++i) {
            temperedProbs[i] = std::pow(actionProbs[i], 1.0 / args.temperature);
            sum += temperedProbs[i];
        }
        for (auto &p : temperedProbs)
            p /= sum;

        // Sample an action.
        int action = sampleAction(temperedProbs);

        // Update the state using a pure transition function.
        StateTransition::getNextState(state, action);

        // Update currentTStates queue with new state
        currentTStates.pop();
        currentTStates.push(state);

        // Update the repetition map with the new state's Zobrist hash.
        uint64_t hash = state.zobrist_hash;
        repetitionMap[hash] += 1;

        // Update state.flags.repeated_state using a helper function
        StateTransition::updateRepeatedStateFlag(state, repetitionMap);

        // TODO: Check what happens here with valid_moves_ptr being nullptr by default
        // Evaluate terminal state.
        auto [value, isTerminal] = GameStatus::evaluateState(state);
        if (isTerminal) {
            // Build training examples from the history.
            std::vector<TrainingExample> examples;
            for (const auto& rec : memory) {
                int outcome = (rec.player == player) ? value : -value;
                auto [history, flags] = getEncodedSnapshotAndFlags(rec.states);
                examples.push_back({StateEncoder::encodeState(history, flags, args.historyLength), rec.actionProbs, outcome});
            }
            return examples;
        }

        player = -player;
    }
}

void AlphaZeroTrainer::train(const std::vector<TrainingExample>& memory) {
    // Stub: Implementation will later convert training batches to LibTorch tensors,
    // perform forward passes, compute losses, backward, and optimizer step.
    std::cout << "[train] Training stub called with " << memory.size() << " examples." << std::endl;
}

void AlphaZeroTrainer::learn() {
    // Stub: Implementation will run full learning iterations.
    std::cout << "[learn] Learning stub called. Iterations: " << args.num_iterations << std::endl;
}

std::pair<std::vector<Chess::HistorySnapshot>, Chess::StateFlags> AlphaZeroTrainer::getEncodedSnapshotAndFlags(const std::vector<Chess::State>& states) {
    std::vector<Chess::HistorySnapshot> history;
    for (const Chess::State& state : states) {
        history.push_back(state.getHistorySnapshot());
    }
    return {history, states[states.size()-1].flags};
}
