#include "State.hpp"
#include <sstream>
#include <random>
#include <chrono>
#include <iomanip>

namespace Chess {

// ------------------- Zobrist Initialization -------------------------
    namespace Zobrist {
        std::array<std::array<uint64_t, 64>, 12> piece_keys;
        uint64_t turn_key;
        std::array<uint64_t, 16> castle_keys;
        std::array<uint64_t, 64> en_passant_keys;

        // Utility: generate a random 64-bit number.
        static uint64_t rand64() {
            static std::mt19937_64 rng(static_cast<uint64_t>(
                                               std::chrono::steady_clock::now().time_since_epoch().count()));
            std::uniform_int_distribution<uint64_t> dist;
            return dist(rng);
        }

        // Initialize all Zobrist keys.
        void init() {
            for (int pt = 0; pt < 12; ++pt)
                for (int sq = 0; sq < 64; ++sq)
                    piece_keys[pt][sq] = rand64();

            turn_key = rand64();

            for (int i = 0; i < 16; ++i)
                castle_keys[i] = rand64();

            for (int i = 0; i < 64; ++i)
                en_passant_keys[i] = rand64();
        }
    } // namespace Zobrist

// ------------------- State Implementation -----------------------------

// Default constructor: sets up the starting position.
    State::State() {
        // Standard starting positions:
        pieces[bb::WHITE_PAWN]   = 0x000000000000ff00ULL;
        pieces[bb::WHITE_KNIGHT] = 0x0000000000000042ULL;
        pieces[bb::WHITE_BISHOP] = 0x0000000000000024ULL;
        pieces[bb::WHITE_ROOK]   = 0x0000000000000081ULL;
        pieces[bb::WHITE_QUEEN]  = 0x0000000000000010ULL;
        pieces[bb::WHITE_KING]   = 0x0000000000000008ULL;

        pieces[bb::BLACK_PAWN]   = 0x00ff000000000000ULL;
        pieces[bb::BLACK_KNIGHT] = 0x4200000000000000ULL;
        pieces[bb::BLACK_BISHOP] = 0x2400000000000000ULL;
        pieces[bb::BLACK_ROOK]   = 0x8100000000000000ULL;
        pieces[bb::BLACK_QUEEN]  = 0x1000000000000000ULL;
        pieces[bb::BLACK_KING]   = 0x0800000000000000ULL;

        // TypeAtSquare
        typeAtSquare = {
                // Rank 8 (White back rank)
                bb::WHITE_ROOK, bb::WHITE_KNIGHT, bb::WHITE_BISHOP, bb::WHITE_QUEEN,
                bb::WHITE_KING, bb::WHITE_BISHOP, bb::WHITE_KNIGHT, bb::WHITE_ROOK,
                // Rank 7 (White pawns)
                bb::WHITE_PAWN, bb::WHITE_PAWN, bb::WHITE_PAWN, bb::WHITE_PAWN,
                bb::WHITE_PAWN, bb::WHITE_PAWN, bb::WHITE_PAWN, bb::WHITE_PAWN,
                // Ranks 6–3 (Empty)
                bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE,
                bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE,
                bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE,
                bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE,
                bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE,
                bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE, bb::NO_PIECE,
                // Rank 2 (Black pawns)
                bb::BLACK_PAWN, bb::BLACK_PAWN, bb::BLACK_PAWN, bb::BLACK_PAWN,
                bb::BLACK_PAWN, bb::BLACK_PAWN, bb::BLACK_PAWN, bb::BLACK_PAWN,
                // Rank 1 (Black back rank)
                bb::BLACK_ROOK, bb::BLACK_KNIGHT, bb::BLACK_BISHOP, bb::BLACK_QUEEN,
                bb::BLACK_KING, bb::BLACK_BISHOP, bb::BLACK_KNIGHT, bb::BLACK_ROOK
        };


        // Initialize flags.
        flags.turn = 0;                // White to move.
        flags.castle_rights = 0xF;       // All castling rights available.
        flags.en_passant = 0;          // No en passant available (or use a sentinel value; adjust logic later).
        flags.repeated_state = 0;      // No repetitions yet.
        flags.half_move_count = 0;
        flags.no_progress_side = 0;    // Default to white.
        flags.total_move_count = 0;

        // Initialize Zobrist keys if needed. (You can call Zobrist::init() once at program startup.)
        static bool zobristInitialized = false;
        if (!zobristInitialized) {
            Zobrist::init();
            zobristInitialized = true;
        }

        // Compute initial Zobrist hash.
        zobrist_hash = computeZobrist();
    }

// Constructor with explicit components.
    State::State(const std::array<uint64_t, 12>& pieces_,
                 const std::array<SquareType, 64>& typeAtSquare_,
                 const StateFlags& flags_)
            : pieces(pieces_), typeAtSquare(typeAtSquare_), flags(flags_)
    {
        zobrist_hash = computeZobrist();
    }

// Compute the Zobrist hash for this state.
    uint64_t State::computeZobrist() const {
        uint64_t hash = 0;

        // For each piece type's bitboard.
        for (int pt = 0; pt < 12; ++pt) {
            uint64_t bb = pieces[pt];
            while (bb) {
                uint64_t sq_bb = bb_utils::pop_lsb(bb);
                int sq = bb_utils::ctz(sq_bb);
                hash ^= Zobrist::piece_keys[pt][sq];
            }
        }
        // XOR turn key if black to move.
        if (flags.turn)
            hash ^= Zobrist::turn_key;

        // XOR in castling rights (4 bits => 16 possibilities).
        hash ^= Zobrist::castle_keys[flags.castle_rights & 0xF];

        // XOR in en passant key if applicable.
        // (Assume that if flags.en_passant is zero then no en passant; adjust as needed)
        if (flags.en_passant != 0)
            hash ^= Zobrist::en_passant_keys[flags.en_passant];

        return hash;
    }

// Returns a history snapshot including the bitboards and repeated_state flag.
    HistorySnapshot State::getHistorySnapshot() const {
        HistorySnapshot snap;
        snap.pieces = pieces;
        snap.repeated_state = flags.repeated_state;
        return snap;
    }

// For debugging: print the board using typeAtSquare (a very simple print function).
    void State::print() const {
        std::ostringstream oss;
        oss << "Turn: " << (flags.turn ? "Black" : "White") << "\n";
        oss << "Castling Rights: " << std::hex << static_cast<int>(flags.castle_rights) << std::dec << "\n";
        oss << "En Passant: " << static_cast<int>(flags.en_passant) << "\n";
        oss << "Half-move Count: " << flags.half_move_count << "\n";
        oss << "Total Move Count: " << flags.total_move_count << "\n";
        oss << "Zobrist Hash: " << std::hex << zobrist_hash << std::dec << "\n\n";

        for (int rank = 7; rank >= 0; --rank) {
            oss << rank + 1 << "  ";
            for (int file = 0; file < 8; ++file) {
                int index = rank * 8 + file;
                SquareType pt = typeAtSquare[index];
                if (pt == bb::NO_PIECE)
                    oss << ". ";
                else
                    oss << static_cast<int>(pt) << " ";  // Print piece type as a number.
            }
            oss << "\n";
        }
        oss << "\n   a b c d e f g h\n";
        std::cout << oss.str();
    }

} // namespace Chess
