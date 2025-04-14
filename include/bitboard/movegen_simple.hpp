#ifndef BITBOARD_MOVEGEN_SIMPLE_HPP
#define BITBOARD_MOVEGEN_SIMPLE_HPP

#include <vector>
#include <cstdint>
#include "bitboard_utils.hpp"
#include "piece_type.hpp"

namespace bb {

/// Generate all resulting piece‐bitboards for knight moves
    std::vector<uint64_t> generate_knight_moves(uint64_t knights,
                                                uint64_t empty,
                                                uint64_t enemy);

/// Generate all resulting piece‐bitboards for king moves
    std::vector<uint64_t> generate_king_moves(uint64_t kings,
                                              uint64_t empty,
                                              uint64_t enemy);

/// Generate all resulting piece‐bitboards for pawn moves (single, double, captures)
/// @param side must be WHITE_PAWN or BLACK_PAWN
    std::vector<uint64_t> generate_pawn_moves(uint64_t pawns,
                                              uint64_t empty,
                                              uint64_t enemy,
                                              PieceType side);

} // namespace bb

#endif // BITBOARD_MOVEGEN_SIMPLE_HPP
