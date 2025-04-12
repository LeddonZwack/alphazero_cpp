#ifndef PAWNS_HPP
#define PAWNS_HPP

#include "SimplePieces.hpp"
#include "Bitboard.hpp"
#include <vector>
#include <cstdint>

namespace Chess {

// Constants for pawn double moves and capture boundaries.
    constexpr uint64_t SECOND_RANK  = 0x000000000000ff00ULL;
    constexpr uint64_t SEVENTH_RANK = 0x00ff000000000000ULL;
    constexpr uint64_t EMPTY_LEFT_SIDE  = 0x7f7f7f7f7f7f7f7fULL;
    constexpr uint64_t EMPTY_RIGHT_SIDE = 0xfefefefefefefefeULL;

    class Pawns : public SimplePieces {
    public:
        // Constructor. If no board is provided, initialize with default pawn positions:
        // WHITE: 0x000000000000ff00, BLACK: 0x00ff000000000000.
        explicit Pawns(PieceType t, uint64_t b = 0ULL)
                : SimplePieces(t, b) {
            if (b == 0ULL) {
                if (t == WHITE_PAWN) {
                    board = 0x000000000000ff00ULL;
                    symbol = "\u265F";  // Unicode white pawn
                } else if (t == BLACK_PAWN) {
                    board = 0x00ff000000000000ULL;
                    symbol = "\u2659";  // Unicode black pawn
                }
            }
        }

        // Single pawn move: shift left 8 for white; for black shift right 8.
        inline uint64_t single_moves(uint64_t b, uint64_t empty) const {
            return (type == WHITE_PAWN) ? ((b << 8) & empty)
                                        : ((b >> 8) & empty);
        }

        // Double pawn move: only for pawns on the starting rank.
        inline uint64_t double_moves(uint64_t b, uint64_t empty) const {
            uint64_t ret = 0;
            if (type == WHITE_PAWN) {
                uint64_t on_second = b & SECOND_RANK;
                uint64_t first_move = (on_second << 8) & empty;
                ret = (first_move << 8) & empty;
            } else {
                uint64_t on_seventh = b & SEVENTH_RANK;
                uint64_t first_move = (on_seventh >> 8) & empty;
                ret = (first_move >> 8) & empty;
            }
            return ret;
        }

        // Left capture: for white shift left 9, for black shift right 7; apply EMPTY_RIGHT_SIDE mask.
        inline uint64_t left_captures(uint64_t b, uint64_t enemy) const {
            return (type == WHITE_PAWN) ?
                   ((b << 9) & enemy & EMPTY_RIGHT_SIDE) :
                   ((b >> 7) & enemy & EMPTY_RIGHT_SIDE);
        }

        // Right capture: for white shift left 7, for black shift right 9; apply EMPTY_LEFT_SIDE mask.
        inline uint64_t right_captures(uint64_t b, uint64_t enemy) const {
            return (type == WHITE_PAWN) ?
                   ((b << 7) & enemy & EMPTY_LEFT_SIDE) :
                   ((b >> 9) & enemy & EMPTY_LEFT_SIDE);
        }

        // Generate all resulting pawn move board states.
        inline std::vector<uint64_t> all_moves(uint64_t empty_squares, uint64_t enemy_pieces) const {
            std::vector<uint64_t> moves;
            uint64_t b = board;

            // SINGLE MOVES
            uint64_t post_move = single_moves(b, empty_squares);
            while (post_move) {
                uint64_t to_square = Bitboard::getLSB(post_move);
                // Compute originating square based on pawn color.
                uint64_t from_square = (type == WHITE_PAWN) ? (to_square >> 8) : (to_square << 8);
                uint64_t new_board = (b | to_square) & Bitboard::complement(from_square);
                moves.push_back(new_board);
                post_move = Bitboard::removeLSB(post_move, to_square);
            }

            // DOUBLE MOVES
            post_move = double_moves(b, empty_squares);
            while (post_move) {
                uint64_t to_square = Bitboard::getLSB(post_move);
                uint64_t from_square = (type == WHITE_PAWN) ? (to_square >> 16) : (to_square << 16);
                uint64_t new_board = (b | to_square) & Bitboard::complement(from_square);
                moves.push_back(new_board);
                post_move = Bitboard::removeLSB(post_move, to_square);
            }

            // LEFT CAPTURE
            post_move = left_captures(b, enemy_pieces);
            while (post_move) {
                uint64_t to_square = Bitboard::getLSB(post_move);
                uint64_t from_square = (type == WHITE_PAWN) ? (to_square >> 9) : (to_square << 7);
                uint64_t new_board = (b | to_square) & Bitboard::complement(from_square);
                moves.push_back(new_board);
                post_move = Bitboard::removeLSB(post_move, to_square);
            }

            // RIGHT CAPTURE
            post_move = right_captures(b, enemy_pieces);
            while (post_move) {
                uint64_t to_square = Bitboard::getLSB(post_move);
                uint64_t from_square = (type == WHITE_PAWN) ? (to_square >> 7) : (to_square << 9);
                uint64_t new_board = (b | to_square) & Bitboard::complement(from_square);
                moves.push_back(new_board);
                post_move = Bitboard::removeLSB(post_move, to_square);
            }

            return moves;
        }
    };

} // namespace Chess

#endif // PAWNS_HPP
