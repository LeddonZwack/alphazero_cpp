#ifndef STATE_HPP
#define STATE_HPP

#include <array>
#include <cstdint>
#include <memory>
#include <iostream>
#include <sstream>
#include "bitboard/bitboard_utils.hpp"   // Your previously provided utilities
#include "bitboard/piece_type.hpp"       // Contains the enum PieceType

namespace Chess {

// We use an uint8_t for square types (even though only 4 bits are needed for values 0–11 and 12 for empty)
    using SquareType = uint8_t;

    enum Color : int {
        WHITE = 0,
        BLACK = 1,
    };

    // Compact game flags packed into a POD struct.
    struct StateFlags {
        unsigned turn              : 1;  // 0 = White, 1 = Black.
        unsigned castle_rights     : 4;  // 4 bits: each bit represents a castling right.
        unsigned en_passant        : 8;  // 8 bits: using a simple encoding (if a square is available, set the bit value to the square index; else, special value).
//        unsigned repeated_state    : 2;  // 2 bits: 00 = first occurrence, 01 = second, 10/11 = third+ occurrence.
        unsigned half_move_count   : 6;  // 6 bits: count for the fifty-move rule (0–63).
        unsigned no_progress_side  : 1;  // 1 bit: indicates which side last made a pawn/capture move (default 0 for white).
        unsigned total_move_count  : 8;  // 8 bits: counts complete moves (0–255).
    };

// History snapshot to be provided to the model: just the bitboards and the repeated_state flag.
    struct HistorySnapshot {
        std::array<uint64_t, 12> pieces;
        unsigned repeated_state : 2;
    };

// The State class itself, which will be stored by value in a Node.
    class State {
    public:
        // 12 bitboards for piece types (indexed via PieceType)
        std::array<uint64_t, 12> pieces;

        // Array to quickly look up the piece type on each square, of length 64.
        std::array<SquareType, 64> typeAtSquare;

        // Packed state flags.
        StateFlags flags;

        // The Zobrist hash for this state.
        uint64_t zobrist_hash;

        // Default constructor: sets up the standard chess starting position.
        State();

        // Constructor with given components.
        State(const std::array<uint64_t, 12>& pieces_,
              const std::array<SquareType, 64>& typeAtSquare_,
              const StateFlags& flags_);

        // Computes and returns the Zobrist hash for this state.
        [[nodiscard]] uint64_t computeZobrist() const;

        // Returns a HistorySnapshot containing the bitboards and the repeated_state flag.
        [[nodiscard]] HistorySnapshot getHistorySnapshot() const;

        // For debugging: print a human-readable board based on typeAtSquare.
        void print() const;
    };

/// Global Zobrist key table and initialization.
    namespace Zobrist {
        // Piece on square: 12 piece types × 64 squares.
        extern std::array<std::array<uint64_t, 64>, 12> piece_keys;
        // Turn to move.
        extern uint64_t turn_key;
        // Castling rights: 16 possibilities.
        extern std::array<uint64_t, 16> castle_keys;
        // En passant square: 64 possibilities.
        extern std::array<uint64_t, 64> en_passant_keys;

        // Initialize the Zobrist table (call once at start-up).
        void init();
    }

} // namespace Chess

#endif // STATE_HPP
