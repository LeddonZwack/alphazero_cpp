#ifndef BITBOARD_PIECETYPE_HPP
#define BITBOARD_PIECETYPE_HPP

#include <cstdint>

namespace bb {

/// Enumerates the 12 piece‐types, for indexing into std::array<uint64_t,12>
    enum PieceType : int {
        WHITE_PAWN   = 0,
        WHITE_KNIGHT = 1,
        WHITE_BISHOP = 2,
        WHITE_ROOK   = 3,
        WHITE_QUEEN  = 4,
        WHITE_KING   = 5,
        BLACK_PAWN   = 6,
        BLACK_KNIGHT = 7,
        BLACK_BISHOP = 8,
        BLACK_ROOK   = 9,
        BLACK_QUEEN  = 10,
        BLACK_KING   = 11,
        NO_PIECE     = 12  ///< use when a square is empty or uninitialized
    };

/// Total number of distinct piece‐types
    static constexpr int NUM_PIECE_TYPES = 12;

/// Helpers to test piece color
    static inline bool is_white(PieceType p) {
        return p >= WHITE_PAWN && p <= WHITE_KING;
    }
    static inline bool is_black(PieceType p) {
        return p >= BLACK_PAWN && p <= BLACK_KING;
    }

} // namespace bb

#endif // BITBOARD_PIECETYPE_HPP
