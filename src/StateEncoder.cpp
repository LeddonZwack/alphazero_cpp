#include "StateEncoder.hpp"

namespace StateEncoder {

    std::array<float, 64> bitboardToPlane(uint64_t bb) {
        std::array<float, 64> plane;
        for (int i = 0; i < 64; ++i) {
            plane[i] = ((bb >> i) & 1ULL) ? 1.0f : 0.0f;
        }
        return plane;
    }

    std::vector<float> encodeState(
            const std::vector<Chess::HistorySnapshot>& history,
            const Chess::StateFlags& flags,
            int historyLength
    ) {
        const int T = historyLength;
        const int perSnapshot = 12 + 2;  // 12 piece planes + 2 repetition planes
        const int L = 1    // color plane
                      + 4  // castling bits
                      + 1  // total move count
                      + 1; // half-move (no-progress) count
        const int totalPlanes = T * perSnapshot + L;
        const int planeSize = 64;

        std::vector<float> out;
        out.reserve(totalPlanes * planeSize);

        // 1) History planes
        for (int t = 0; t < T; ++t) {
            const auto& snap = history[t];
            // 12 piece bitboards
            for (int pt = 0; pt < 12; ++pt) {
                auto plane = bitboardToPlane(snap.pieces[pt]);
                out.insert(out.end(), plane.begin(), plane.end());
            }
            // 2 repetition planes (bit0, bit1)
            {
                std::array<float,64> rep0, rep1;
                float b0 = (snap.repeated_state & 0b01) ? 1.0f : 0.0f;
                float b1 = (snap.repeated_state & 0b10) ? 1.0f : 0.0f;
                rep0.fill(b0);
                rep1.fill(b1);
                out.insert(out.end(), rep0.begin(), rep0.end());
                out.insert(out.end(), rep1.begin(), rep1.end());
            }
        }

        // 2) L planes for current flags

        // a) Color plane: 1.0 if White to move, 0.0 if Black
        {
            std::array<float,64> colorPlane;
            float v = (flags.turn == 0) ? 1.0f : 0.0f;
            colorPlane.fill(v);
            out.insert(out.end(), colorPlane.begin(), colorPlane.end());
        }

        // b) Castling rights: one plane per bit (bit0 = white queen-side, bit1 = white king-side, bit2 = black queen-side, bit3 = black king-side)
        for (int bit = 0; bit < 4; ++bit) {
            std::array<float,64> plane;
            float v = ((flags.castle_rights >> bit) & 1) ? 1.0f : 0.0f;
            plane.fill(v);
            out.insert(out.end(), plane.begin(), plane.end());
        }

        // c) Total move count plane (normalized by 100.0f)
        {
            std::array<float,64> plane;
            float v = static_cast<float>(flags.total_move_count) / 100.0f;
            plane.fill(v);
            out.insert(out.end(), plane.begin(), plane.end());
        }

        // d) Half-move (no-progress) count plane (normalized by 50.0f)
        {
            std::array<float,64> plane;
            float v = static_cast<float>(flags.half_move_count) / 50.0f;
            plane.fill(v);
            out.insert(out.end(), plane.begin(), plane.end());
        }

        return out;
    }

} // namespace StateEncoder
