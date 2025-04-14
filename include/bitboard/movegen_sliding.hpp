#ifndef BITBOARD_MOVEGEN_SLIDING_HPP
#define BITBOARD_MOVEGEN_SLIDING_HPP

#include <vector>
#include <cstdint>

namespace bb {

/// Generate all resulting piece bitboards for rook moves.
/// This is a naïve loop‐based approach.
/// TODO: Optimize with magic bitboards in future.
    std::vector<uint64_t> generate_rook_moves(uint64_t rooks,
                                              uint64_t empty,
                                              uint64_t enemy);

/// Generate all resulting piece bitboards for bishop moves.
/// This is a naïve loop‐based approach.
/// TODO: Optimize with magic bitboards in future.
    std::vector<uint64_t> generate_bishop_moves(uint64_t bishops,
                                                uint64_t empty,
                                                uint64_t enemy);

/// Generate all resulting piece bitboards for queen moves
/// by combining rook and bishop moves.
    std::vector<uint64_t> generate_queen_moves(uint64_t queens,
                                               uint64_t empty,
                                               uint64_t enemy);

} // namespace bb

#endif // BITBOARD_MOVEGEN_SLIDING_HPP
