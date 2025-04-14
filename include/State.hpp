#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include <array>
#include <deque>
#include <unordered_map>
#include <memory>
#include <string>
#include <cstdint>

#include "Pieces/Pieces.hpp"  // Base class for pieces

namespace Chess {

    // Alias for a 64‐bit board.
    using BB = uint64_t;

//    // Player constants
//    constexpr int WHITE = 1;
//    constexpr int BLACK = -1;
//
//    // Enumeration for the 12 piece types.
//    enum PieceType {
//        WHITE_PAWN   = 0,
//        WHITE_KNIGHT = 1,
//        WHITE_BISHOP = 2,
//        WHITE_ROOK   = 3,
//        WHITE_QUEEN  = 4,
//        WHITE_KING   = 5,
//        BLACK_PAWN   = 6,
//        BLACK_KNIGHT = 7,
//        BLACK_BISHOP = 8,
//        BLACK_ROOK   = 9,
//        BLACK_QUEEN  = 10,
//        BLACK_KING   = 11
//    };

    // Instead of HistoryEntry storing a deep copy of the boards,
    // we now store shared pointers to previous State snapshots.
    // This assumes that once a state is stored, it is never modified.
    using StateHistory = std::deque<std::weak_ptr<class State>>;

    class State : public std::enable_shared_from_this<State> {
    public:
        // Array of piece objects, one per piece type.
        // We use unique_ptr to store polymorphic Pieces. Each piece is created with its proper type.
        std::array<std::unique_ptr<Pieces>, 12> pieces;
        // For each square 0..63, store the piece type if occupied, or -1 if empty.
        std::array<int, 64> typeAtSquare;

        // Turn: WHITE (1) means White to move, BLACK (-1) means Black.
        int turn;

        // Castling rights: bit‐field (e.g., 0x0F for all rights available).
        uint8_t castle_flags;

        // En passant flag (a bitboard where a 1 marks the square available for en passant capture)
        BB en_passant_flags;

        // Repetition flag.
        uint8_t repeated_moves_flags;

        // Counters for no-progress and total move counts.
        int no_progress_flags;
        int no_progress_color_flags;
        int total_move_flags;

        // History: a queue of previous state snapshots.
        StateHistory history;

        // Maximum number of history entries to store.
        int historyLength;

        // Repetition counter.
        std::unordered_map<std::string, int> repeated_moves;

        // Constructor: initializes state to the standard chess starting position.
        explicit State(int historyLen = 5);

        // Copy constructor.
        State(const State &other);

        // (Optional) Virtual destructor.
        virtual ~State() = default;

        std::shared_ptr<State> returnStatePtr();

    private:
        // Helper: initialize history.
        void initializeHistory();
    };

} // namespace Chess

#endif // GAMESTATE_HPP
