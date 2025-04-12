#ifndef KINGS_HPP
#define KINGS_HPP

#include "SimplePieces.hpp"
#include <vector>
#include <cstdint>
#include <algorithm>

namespace Chess {

    class Kings : public SimplePieces {
    public:
        // Barrier masks for king moves.
        static constexpr uint64_t EMPTY_LEFT_SIDE  = 0x7f7f7f7f7f7f7f7fULL;
        static constexpr uint64_t EMPTY_RIGHT_SIDE = 0xfefefefefefefefeULL;

        // Define the type for a move function pointer.
        // A move function takes a bitboard, an empty squares bitboard, and an enemy bitboard, returns a new bitboard.
        using MoveFunction = uint64_t (*)(uint64_t, uint64_t, uint64_t);

        // Constructor: if no board is provided, initialize with default positions.
        explicit Kings(PieceType t, uint64_t b = 0ULL)
                : SimplePieces(t, b) {
            if (b == 0ULL) {
                if (t == WHITE_KING) {
                    board = 0x0000000000000008ULL;
                    symbol = "\u265A";  // Unicode white king
                } else if (t == BLACK_KING) {
                    board = 0x0800000000000000ULL;
                    symbol = "\u2654";  // Unicode black king
                }
            }
        }

        // Static movement functions:
        static inline uint64_t left(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                    uint64_t enemy = 0xffffffffffffffffULL) {
            return (b << 1) & (empty | enemy) & EMPTY_RIGHT_SIDE;
        }

        static inline uint64_t right(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                     uint64_t enemy = 0xffffffffffffffffULL) {
            return (b >> 1) & (empty | enemy) & EMPTY_LEFT_SIDE;
        }

        static inline uint64_t up(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                  uint64_t enemy = 0xffffffffffffffffULL) {
            return (b << 8) & (empty | enemy);
        }

        static inline uint64_t down(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                    uint64_t enemy = 0xffffffffffffffffULL) {
            return (b >> 8) & (empty | enemy);
        }

        static inline uint64_t up_left(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                       uint64_t enemy = 0xffffffffffffffffULL) {
            return (b << 9) & (empty | enemy) & EMPTY_RIGHT_SIDE;
        }

        static inline uint64_t up_right(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                        uint64_t enemy = 0xffffffffffffffffULL) {
            return (b << 7) & (empty | enemy) & EMPTY_LEFT_SIDE;
        }

        static inline uint64_t down_left(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                         uint64_t enemy = 0xffffffffffffffffULL) {
            return (b >> 7) & (empty | enemy) & EMPTY_RIGHT_SIDE;
        }

        static inline uint64_t down_right(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                          uint64_t enemy = 0xffffffffffffffffULL) {
            return (b >> 9) & (empty | enemy) & EMPTY_LEFT_SIDE;
        }

    private:
        // Define the order for move functions:
        // 0: left, 1: right, 2: up, 3: down, 4: up_left, 5: up_right, 6: down_left, 7: down_right.
        static inline const MoveFunction moveFunctions[8] = {
                &Kings::left,
                &Kings::right,
                &Kings::up,
                &Kings::down,
                &Kings::up_left,
                &Kings::up_right,
                &Kings::down_left,
                &Kings::down_right
        };

        // Define the opposites: left↔right, up↔down, up_left↔down_right, up_right↔down_left.
        static inline const MoveFunction oppositeFunctions[8] = {
                &Kings::right,     // opposite of left
                &Kings::left,      // opposite of right
                &Kings::down,      // opposite of up
                &Kings::up,        // opposite of down
                &Kings::down_right, // opposite of up_left
                &Kings::down_left,  // opposite of up_right
                &Kings::up_right,   // opposite of down_left
                &Kings::up_left     // opposite of down_right
        };

    public:
        // Generates all legal moves for the king (as new bitboards) given empty squares and enemy pieces.
        inline std::vector<uint64_t> all_moves(uint64_t empty_squares,
                                               uint64_t enemy_pieces) const {
            std::vector<uint64_t> result;
            for (int i = 0; i < 8; ++i) {
                uint64_t post_move = moveFunctions[i](board, empty_squares, enemy_pieces);
                while (post_move) {
                    uint64_t to_square = Bitboard::getLSB(post_move);
                    // Reverse the move via the opposite function
                    uint64_t from_square = oppositeFunctions[i](to_square, empty_squares, enemy_pieces);
                    uint64_t new_board = (board | to_square) & Bitboard::complement(from_square);
                    result.push_back(new_board);
                    post_move = Bitboard::removeLSB(post_move, to_square);
                }
            }
            return result;
        }
    };

} // namespace Chess

#endif // KINGS_HPP
