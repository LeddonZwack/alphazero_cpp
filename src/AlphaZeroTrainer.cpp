#include "AlphaZeroTrainer.hpp"
#include "MCTS.hpp"            // Our MCTS module.
#include "StateTransition.hpp" // Provides getCopyNextState, etc.
#include "GameStatus.hpp"      // Provides evaluateState.
#include "ModelInterface.hpp"

#include <fstream>     // for std::ofstream
#include <filesystem>  // for std::filesystem
#include <chrono>
#include <ctime>
#include <iomanip>     // for std::put_time
#include <sstream>     // for std::ostringstream

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
AlphaZeroTrainer::AlphaZeroTrainer(ModelInterface& modelInterface,
                                   TrainerArgs trainerArgs,
                                   GameConfig gameConfig)
        : modelIf_(modelInterface),
          trainerArgs_(std::move(trainerArgs)),
          gameConfig_(std::move(gameConfig)) {
    // Constructor body if needed
}

std::vector<TrainingExample> AlphaZeroTrainer::selfPlay() {
    // Local structure to record self-play history.
    struct SelfPlayRecord {
        std::vector<Chess::State> states;
        std::array<float, ACTION_SIZE> actionProbs;
        int player; // +1, -1
    };

    // Starting player.
    int player = 1;
    // Initialize the state using the default constructor (starting position).
    Chess::State state;
    // Instantiate a local MCTS searcher for this move.
    MCTS::MCTS mctsSearcher(trainerArgs_, modelIf_); // Construct with args or configuration as needed.

    // Vector to hold memory
    std::vector<SelfPlayRecord> memory;
    // Create an external repetition map: maps a state's Zobrist hash to the count.
    std::unordered_map<uint64_t, uint8_t> repetitionMap;
    // Create a queue to hold previous T states
    std::queue<Chess::State> currentTStates;

    // Populate queue with initial state
    for (int i = 0; i < trainerArgs_.historyLength; ++i) {
        currentTStates.push(state);
    }

    // Insert root state's hash.
    repetitionMap[state.zobrist_hash] = 1;

    int counter = 0;

//    std::cout << "Printing current state " << counter << " board \n";
//    state.validateAndPrintBoard();

    // Each self-play episode runs until game termination.
    while (true) {
        counter++;
//        std::cout << "I'm move: " << counter << " in self play \n";
//        state.print();
        // Call mctsSearcher.search(state, repetitionMap)
        // Assume that mctsSearcher.search accepts the current state and a reference to the repetition map.
        std::array<float, ACTION_SIZE> actionProbs = mctsSearcher.search(state, repetitionMap);

        // Create a record and fill it from the queue
        SelfPlayRecord record;

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
            temperedProbs[i] = std::pow(actionProbs[i], 1.0 / trainerArgs_.temperature);
            sum += temperedProbs[i];
        }
        for (auto &p : temperedProbs)
            p /= sum;

        // Sample an action.
        int action = sampleAction(temperedProbs);

        // Update the state using a pure transition function and get clearMap flag
        bool clearMap = StateTransition::getNextState(state, action);

//        std::cout << "Printing current state " << counter << " board \n";
//        state.validateAndPrintBoard();

        // If we should clear the repetition map, do so
        if (clearMap) repetitionMap.clear();

        // Update currentTStates queue with new state
        currentTStates.pop();
        currentTStates.push(state);

        // Update the repetition map with the new state's Zobrist hash.
        repetitionMap[state.zobrist_hash] += 1;

        // Update state.flags.repeated_state using a helper function
        StateTransition::updateRepeatedStateFlag(state, repetitionMap.at(state.zobrist_hash));

        // TODO: Check what happens here with valid_moves_ptr being nullptr by default
        // Evaluate terminal state.
        auto [value, isTerminal] = GameStatus::evaluateState(state);
        if (isTerminal) {
            // Build training examples from the history.
            std::vector<TrainingExample> examples;
            for (const auto& rec : memory) {
                int outcome = (rec.player == player) ? value : -value;
                auto [history, flags] = ModelInterface::getEncodedSnapshotAndFlags(rec.states);
                examples.push_back({StateEncoder::encodeState(history, flags, trainerArgs_.historyLength), rec.actionProbs, outcome});
            }
            return examples;
        }

        player = -player;
    }
}

// Train on one iteration’s worth of self‑play data.
void AlphaZeroTrainer::train(const std::vector<TrainingExample>& memory) {
    int N = (int)memory.size();
    if (N == 0) {
        std::cerr << "[train] Warning: zero training examples\n";
        return;
    }

    // Shuffle copy
    auto examples = memory;
    std::mt19937_64 rng{std::random_device{}()};
    std::shuffle(examples.begin(), examples.end(), rng);

    // Batch over epochs
    for (int epoch = 1; epoch <= trainerArgs_.num_epochs; ++epoch) {
        std::cout << "[train] Epoch " << epoch
                  << "/" << trainerArgs_.num_epochs
                  << " — " << N << " examples"
                  << " in " << trainerArgs_.batch_size << "-sized batches\n";

        for (int start = 0; start < N; start += trainerArgs_.batch_size) {
            int end = std::min(start + trainerArgs_.batch_size, N);
            std::vector<TrainingExample> batch(
                    examples.begin() + start,
                    examples.begin() + end
            );
            modelIf_.trainBatch(batch);
        }
    }
}

// Helper to get timestamp string
std::string getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Call this AFTER saving your checkpoint
void AlphaZeroTrainer::logCheckpoint(int iteration) {
    namespace fs = std::filesystem;

    // Find the true project root (go up from the build dir)
    fs::path currentPath = fs::current_path();      // e.g., cmake-build-release/
    fs::path projectRoot = currentPath.parent_path(); // Go up one level to alphazero_cpp/

    // Path to checkpoints folder inside project
    fs::path logDir = projectRoot / "logs";
    if (!fs::exists(logDir)) {
        fs::create_directory(logDir);
    }

    // Open (or create) the file in append mode
    fs::path logFilePath = logDir / "checkpoint_log.txt";
    std::ofstream logFile(logFilePath, std::ios::app);

    if (!logFile.is_open()) {
        std::cerr << "[logCheckpoint] Failed to open checkpoint_log.txt for writing.\n";
        return;
    }

    std::string timestamp = getCurrentTimeString();

    logFile << "Iteration: " << iteration << " | Timestamp: " << timestamp << "\n";

    logFile.close();
}

// The overall learning loop.
void AlphaZeroTrainer::learn() {
    std::cout << "[learn] Starting learning: "
              << trainerArgs_.num_iterations << " iterations, "
              << trainerArgs_.num_selfPlay_iterations << " games/iter\n";

    std::cout << "Logging initial start time\n";
    logCheckpoint(0);

    for (int iter = 1; iter <= trainerArgs_.num_iterations; ++iter) {
        std::cout << "\n[learn] === Iteration " << iter
                  << " of " << trainerArgs_.num_iterations << " ===\n";

        // 1) Self‑play: gather multiple full-game examples
        std::vector<TrainingExample> memory;
        for (int g = 1; g <= trainerArgs_.num_selfPlay_iterations; ++g) {
            auto gameData = selfPlay();                     // runs until terminal
            memory.insert(memory.end(),
                          gameData.begin(), gameData.end());
            std::cout << "[learn]  Collected " << gameData.size()
                      << " examples from game " << g << "\n";
        }
        std::cout << "[learn] Total examples: " << memory.size() << "\n";

        // 2) Train on that memory
        train(memory);

        // Save model and optimizer here
        modelIf_.saveCheckpoint(iter);
        logCheckpoint(iter);
        std::cout << "[learn] Saved checkpoint for iteration " << iter << "\n";
    }

    std::cout << "[learn] All " << trainerArgs_.num_iterations
              << " iterations complete\n";
}

