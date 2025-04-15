#ifndef STATE_ENCODER_HPP
#define STATE_ENCODER_HPP

#include <vector>
#include <array>
#include <cstdint>
#include "State.hpp"

namespace StateEncoder {

/// Convert a 64‑bit bitboard into an 8×8 plane of floats (row‑major).
/// We map bit 0 → index 0, bit 1 → index 1, …, bit 63 → index 63.
    std::array<float, 64> bitboardToPlane(uint64_t bb);

/// Encode a sequence of history snapshots plus current flags into a single
/// float vector of shape [(T * 14) + 7] × 64, where:
///  - For each snapshot: 12 piece planes + 2 repetition planes = 14 planes.
///  - Then 7 “L” planes: color, 4 castling bits, total‑move, half‑move.
/// @param history         Vector of length T of HistorySnapshot (pieces + repeated_state).
/// @param flags           Current StateFlags (for L planes).
/// @param historyLength   T, the length of the history vector.
/// @returns               A flat vector of size ((T*14 + 7)*64).
    std::vector<float> encodeState(
            const std::vector<Chess::HistorySnapshot>& history,
            const Chess::StateFlags& flags,
            int historyLength
    );

} // namespace StateEncoder

#endif // STATE_ENCODER_HPP
