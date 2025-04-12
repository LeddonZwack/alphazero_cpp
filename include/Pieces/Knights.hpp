#ifndef KNIGHTS_HPP
#define KNIGHTS_HPP

#include "SimplePieces.hpp"
#include <vector>
#include <cstdint>
#include <algorithm>

namespace Chess {

    class Knights : public SimplePieces {
    public:
        // Barrier constants for knight moves.
        static constexpr uint64_t EMPTY_LEFT_SIDE      = 0x7f7f7f7f7f7f7f7fULL;
        static constexpr uint64_t EMPTY_RIGHT_SIDE     = 0xfefefefefefefefeULL;
        static constexpr uint64_t EMPTY_TWO_LEFT_SIDE  = 0x3f3f3f3f3f3f3f3fULL;
        static constexpr uint64_t EMPTY_TWO_RIGHT_SIDE = 0xfcfcfcfcfcfcfcfcULL;

        // Type for move function pointer.
        using MoveFunction = uint64_t (*)(uint64_t, uint64_t, uint64_t);

        explicit Knights(PieceType t, uint64_t b = 0ULL)
                : SimplePieces(t, b) {
            if (b == 0ULL) {
                if (t == WHITE_KNIGHT) {
                    board = 0x0000000000000042ULL;
                    symbol = "\u265E";  // Unicode white knight
                } else if (t == BLACK_KNIGHT) {
                    board = 0x4200000000000000ULL;
                    symbol = "\u2658";  // Unicode black knight
                }
            }
        }

        // Define knight movement functions.
        static inline uint64_t up_left(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                       uint64_t enemy = 0xffffffffffffffffULL) {
            return (b << 17) & (empty | enemy) & EMPTY_RIGHT_SIDE;
        }
        static inline uint64_t up_right(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                        uint64_t enemy = 0xffffffffffffffffULL) {
            return (b << 15) & (empty | enemy) & EMPTY_LEFT_SIDE;
        }
        static inline uint64_t down_left(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                         uint64_t enemy = 0xffffffffffffffffULL) {
            return (b >> 15) & (empty | enemy) & EMPTY_RIGHT_SIDE;
        }
        static inline uint64_t down_right(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                          uint64_t enemy = 0xffffffffffffffffULL) {
            return (b >> 17) & (empty | enemy) & EMPTY_LEFT_SIDE;
        }
        static inline uint64_t left_up(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                       uint64_t enemy = 0xffffffffffffffffULL) {
            return (b << 10) & (empty | enemy) & EMPTY_TWO_RIGHT_SIDE;
        }
        static inline uint64_t left_down(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                         uint64_t enemy = 0xffffffffffffffffULL) {
            return (b >> 6) & (empty | enemy) & EMPTY_TWO_RIGHT_SIDE;
        }
        static inline uint64_t right_up(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                        uint64_t enemy = 0xffffffffffffffffULL) {
            return (b << 6) & (empty | enemy) & EMPTY_TWO_LEFT_SIDE;
        }
        static inline uint64_t right_down(uint64_t b, uint64_t empty = 0xffffffffffffffffULL,
                                          uint64_t enemy = 0xffffffffffffffffULL) {
            return (b >> 10) & (empty | enemy) & EMPTY_TWO_LEFT_SIDE;
        }

    private:
        // Order for knight move functions:
        // 0: up_left, 1: up_right, 2: down_left, 3: down_right, 4: left_up, 5: left_down, 6: right_up, 7: right_down.
        static inline const MoveFunction moveFunctions[8] = {
                &Knights::up_left,
                &Knights::up_right,
                &Knights::down_left,
                &Knights::down_right,
                &Knights::left_up,
                &Knights::left_down,
                &Knights::right_up,
                &Knights::right_down
        };

        // Opposite functions mapping:
        // up_left   -> down_right,
        // up_right  -> down_left,
        // down_left -> up_right,
        // down_right-> up_left,
        // left_up   -> right_down,
        // left_down -> right_up,
        // right_up  -> left_down,
        // right_down-> left_up.
        static inline const MoveFunction oppositeFunctions[8] = {
                &Knights::down_right,
                &Knights::down_left,
                &Knights::up_right,
                &Knights::up_left,
                &Knights::right_down,
                &Knights::right_up,
                &Knights::left_down,
                &Knights::left_up
        };

    public:
        // Generate all moves for the knight.
        inline std::vector<uint64_t> all_moves(uint64_t empty_squares,
                                               uint64_t enemy_pieces) const {
            std::vector<uint64_t> result;
            for (int i = 0; i < 8; ++i) {
                uint64_t post_move = moveFunctions[i](board, empty_squares, enemy_pieces);
                while (post_move) {
                    uint64_t to_square = Bitboard::getLSB(post_move);
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

#endif // KNIGHTS_HPP
