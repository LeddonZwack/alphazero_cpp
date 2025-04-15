#include "bitboard/movegen_simple.hpp"
#include <stdexcept>

namespace bb {

//— Knight moves --------------------------------------------------------------
    std::vector<uint64_t> generate_knight_moves(uint64_t knights,
                                                uint64_t empty,
                                                uint64_t enemy) {
        std::vector<uint64_t> out;
        uint64_t b = knights;

        // masks to prevent wrap‑around
        static constexpr uint64_t MASK_L1 = 0x7f7f7f7f7f7f7f7fULL;
        static constexpr uint64_t MASK_R1 = 0xfefefefefefefefeULL;
        static constexpr uint64_t MASK_L2 = 0x3f3f3f3f3f3f3f3fULL;
        static constexpr uint64_t MASK_R2 = 0xfcfcfcfcfcfcfcfcULL;

        while (b) {
            uint64_t from_bb = bb_utils::pop_lsb(b);

            struct { int shift; uint64_t mask; } dirs[] = {
                    { +17, MASK_R1 }, { +15, MASK_L1 },
                    { -17, MASK_L1 }, { -15, MASK_R1 },
                    { +10, MASK_R2 }, { +6,  MASK_L2 },
                    { -10, MASK_L2 }, { -6,  MASK_R2 }
            };

            for (auto &d : dirs) {
                uint64_t targets;
                if (d.shift > 0)
                    targets = (from_bb << d.shift);
                else
                    targets = (from_bb >> (-d.shift));

                targets &= (empty | enemy) & d.mask;

                while (targets) {
                    uint64_t to_bb = bb_utils::pop_lsb(targets);
                    // opposite shift to get the from‐square bitboard
                    uint64_t recovered_from = (d.shift > 0)
                                              ? (to_bb >> d.shift)
                                              : (to_bb << (-d.shift));
                    // new knight‐bitboard: remove old, add new
                    uint64_t new_bb = (knights | to_bb) & bb_utils::complement(recovered_from);
                    out.push_back(new_bb);
                }
            }
        }
        return out;
    }

//— King moves ---------------------------------------------------------------
    std::vector<uint64_t> generate_king_moves(uint64_t kings,
                                              uint64_t empty,
                                              uint64_t enemy) {
        std::vector<uint64_t> out;
        uint64_t b = kings;

        // masks to prevent wrap‑around
        static constexpr uint64_t MASK_L = 0x7f7f7f7f7f7f7f7fULL;
        static constexpr uint64_t MASK_R = 0xfefefefefefefefeULL;

        while (b) {
            uint64_t from_bb = bb_utils::pop_lsb(b);

            struct { int shift; uint64_t mask; } dirs[] = {
                    { +1,  MASK_R }, { -1,  MASK_L },
                    { +8,  UINT64_C(0xFFFFFFFFFFFFFFFF) },
                    { -8,  UINT64_C(0xFFFFFFFFFFFFFFFF) },
                    { +9,  MASK_R }, { -7,  MASK_R },
                    { +7,  MASK_L }, { -9,  MASK_L }
            };

            for (auto &d : dirs) {
                uint64_t targets;
                if (d.shift > 0)
                    targets = (from_bb << d.shift);
                else
                    targets = (from_bb >> (-d.shift));

                targets &= (empty | enemy) & d.mask;

                while (targets) {
                    uint64_t to_bb = bb_utils::pop_lsb(targets);
                    uint64_t recovered_from = (d.shift > 0)
                                              ? (to_bb >> d.shift)
                                              : (to_bb << (-d.shift));
                    uint64_t new_bb = (kings | to_bb) & bb_utils::complement(recovered_from);
                    out.push_back(new_bb);
                }
            }
        }
        return out;
    }

//— Pawn moves ---------------------------------------------------------------
    std::vector<uint64_t> generate_pawn_moves(uint64_t pawns,
                                              uint64_t empty,
                                              uint64_t enemy,
                                              PieceType side) {
        if (side != WHITE_PAWN && side != BLACK_PAWN)
            throw std::invalid_argument("generate_pawn_moves: side must be WHITE_PAWN or BLACK_PAWN");

        std::vector<uint64_t> out;
        uint64_t b = pawns;

        static constexpr uint64_t RANK2 = 0x000000000000FF00ULL;
        static constexpr uint64_t RANK7 = 0x00FF000000000000ULL;
        static constexpr uint64_t MASK_L = 0x7f7f7f7f7f7f7f7fULL;
        static constexpr uint64_t MASK_R = 0xfefefefefefefefeULL;

        bool isWhite = (side == WHITE_PAWN);

        // Single pushes
        {
            uint64_t single = isWhite
                              ? ((pawns << 8) & empty)
                              : ((pawns >> 8) & empty);

            while (single) {
                uint64_t to_bb = bb_utils::pop_lsb(single);
                uint64_t from_bb = isWhite ? (to_bb >> 8) : (to_bb << 8);
                uint64_t new_bb = (pawns | to_bb) & bb_utils::complement(from_bb);
                out.push_back(new_bb);
            }
        }

        // Double pushes
        {
            uint64_t rank_mask = isWhite ? RANK2 : RANK7;
            uint64_t on_rank    = pawns & rank_mask;
            uint64_t first_step = isWhite
                                  ? ((on_rank << 8) & empty)
                                  : ((on_rank >> 8) & empty);
            uint64_t dbl = isWhite
                           ? ((first_step << 8) & empty)
                           : ((first_step >> 8) & empty);

            while (dbl) {
                uint64_t to_bb = bb_utils::pop_lsb(dbl);
                uint64_t from_bb = isWhite ? (to_bb >> 16) : (to_bb << 16);
                uint64_t new_bb = (pawns | to_bb) & bb_utils::complement(from_bb);
                out.push_back(new_bb);
            }
        }

        // Captures
        struct { int shift; uint64_t mask; } caps[] = {
                { +7, MASK_L }, { +9, MASK_R },
                { -7, MASK_R }, { -9, MASK_L }
        };

        for (auto &c : caps) {
            // only two are valid for each color
            if ((isWhite && (c.shift == -7 || c.shift == -9)) ||
                (!isWhite && (c.shift == +7 || c.shift == +9)))
                continue;

            uint64_t cap_bb;
            if (c.shift > 0)
                cap_bb = ((pawns << c.shift) & enemy & c.mask);
            else
                cap_bb = ((pawns >> (-c.shift)) & enemy & c.mask);

            while (cap_bb) {
                uint64_t to_bb = bb_utils::pop_lsb(cap_bb);
                uint64_t from_bb = (c.shift > 0)
                                   ? (to_bb >> c.shift)
                                   : (to_bb << (-c.shift));
                uint64_t new_bb = (pawns | to_bb) & bb_utils::complement(from_bb);
                out.push_back(new_bb);
            }
        }

        // TODO: handle en‑passant, promotions
        return out;
    }

} // namespace bb
