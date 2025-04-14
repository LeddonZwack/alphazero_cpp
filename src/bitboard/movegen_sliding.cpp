#include "Bitboard/movegen_sliding.hpp"
#include "Bitboard/bitboard_utils.hpp"
#include <vector>

namespace bb {

// Helper functions to convert an index (0-63) into its rank and file.
    static inline int get_rank(int index) {
        return index / 8;
    }
    static inline int get_file(int index) {
        return index % 8;
    }

// Convert a rank and file to a bitboard with only that square set.
    static inline uint64_t square_to_bit(int rank, int file) {
        return uint64_t(1) << (rank * 8 + file);
    }

// Structure to represent movement directions.
    struct Direction {
        int d_rank;
        int d_file;
    };

// Helper: for a single sliding piece (which is represented by one bit),
// generate destination squares in the given direction.
    static std::vector<uint64_t> generate_moves_in_direction(uint64_t piece_bit,
                                                             const Direction& dir,
                                                             uint64_t empty,
                                                             uint64_t enemy) {
        std::vector<uint64_t> moves;
        int index = bb_utils::ctz(piece_bit); // Assumes piece_bit has one bit set.
        int rank = get_rank(index);
        int file = get_file(index);

        // Slide step by step.
        while (true) {
            rank += dir.d_rank;
            file += dir.d_file;
            if (rank < 0 || rank >= 8 || file < 0 || file >= 8)
                break; // Exceeded board boundaries.
            uint64_t square_bit = square_to_bit(rank, file);
            // If the square is empty, it’s a valid move; continue sliding.
            if (empty & square_bit) {
                moves.push_back(square_bit);
            } else if (enemy & square_bit) {
                // Can capture enemy; then stop.
                moves.push_back(square_bit);
                break;
            } else {
                // Own piece blocks further sliding.
                break;
            }
        }
        return moves;
    }

// Core function: for all sliding pieces in the bitboard,
// generate all moves in the provided directions.
    static std::vector<uint64_t> generate_sliding_moves(uint64_t pieces,
                                                        uint64_t empty,
                                                        uint64_t enemy,
                                                        const std::vector<Direction>& dirs) {
        std::vector<uint64_t> results;
        uint64_t temp = pieces;
        while (temp) {
            // Isolate one piece.
            uint64_t piece = bb_utils::pop_lsb(temp);
            // Base: state of the given piece type with the moving piece removed.
            uint64_t base = pieces & bb_utils::complement(piece);
            // Generate moves for each allowed direction.
            for (const auto& d : dirs) {
                std::vector<uint64_t> moves = generate_moves_in_direction(piece, d, empty, enemy);
                for (auto dest : moves) {
                    // Create a new bitboard state for this piece type:
                    // remove the source square and add the destination.
                    uint64_t new_state = base | dest;
                    results.push_back(new_state);
                }
            }
        }
        return results;
    }

// Rook moves: vertical and horizontal.
    std::vector<uint64_t> generate_rook_moves(uint64_t rooks,
                                              uint64_t empty,
                                              uint64_t enemy) {
        // Rook directions: up, down, right, left.
        std::vector<Direction> rook_dirs = {
                {1, 0},   // up
                {-1, 0},  // down
                {0, 1},   // right
                {0, -1}   // left
        };
        return generate_sliding_moves(rooks, empty, enemy, rook_dirs);
    }

// Bishop moves: 4 diagonal directions.
    std::vector<uint64_t> generate_bishop_moves(uint64_t bishops,
                                                uint64_t empty,
                                                uint64_t enemy) {
        std::vector<Direction> bishop_dirs = {
                {1, 1},    // up-right
                {1, -1},   // up-left
                {-1, 1},   // down-right
                {-1, -1}   // down-left
        };
        return generate_sliding_moves(bishops, empty, enemy, bishop_dirs);
    }

// Queen moves: combination of rook and bishop moves.
    std::vector<uint64_t> generate_queen_moves(uint64_t queens,
                                               uint64_t empty,
                                               uint64_t enemy) {
        std::vector<uint64_t> moves;
        std::vector<uint64_t> rook_moves = generate_rook_moves(queens, empty, enemy);
        std::vector<uint64_t> bishop_moves = generate_bishop_moves(queens, empty, enemy);
        moves.insert(moves.end(), rook_moves.begin(), rook_moves.end());
        moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());
        return moves;
    }

} // namespace bb
