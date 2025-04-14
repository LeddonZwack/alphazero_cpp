#ifndef MOVEMENT_HPP
#define MOVEMENT_HPP

#include <cstdint>
#include <array>

namespace Chess {

    using Bitboard  = uint64_t;
    using MoveFunc  = Bitboard(*)(Bitboard);

//------------------------------------------------------------------------------
// 1) Straight‐line queen/rook/bishop moves (0–55)
//------------------------------------------------------------------------------
// up (north)
    inline Bitboard m_up1   (Bitboard b) { return b <<  8; }
    inline Bitboard m_up2   (Bitboard b) { return b << 16; }
    inline Bitboard m_up3   (Bitboard b) { return b << 24; }
    inline Bitboard m_up4   (Bitboard b) { return b << 32; }
    inline Bitboard m_up5   (Bitboard b) { return b << 40; }
    inline Bitboard m_up6   (Bitboard b) { return b << 48; }
    inline Bitboard m_up7   (Bitboard b) { return b << 56; }

// up‐right (north‐east)
    inline Bitboard m_ur1   (Bitboard b) { return b <<  7; }
    inline Bitboard m_ur2   (Bitboard b) { return b << 14; }
    inline Bitboard m_ur3   (Bitboard b) { return b << 21; }
    inline Bitboard m_ur4   (Bitboard b) { return b << 28; }
    inline Bitboard m_ur5   (Bitboard b) { return b << 35; }
    inline Bitboard m_ur6   (Bitboard b) { return b << 42; }
    inline Bitboard m_ur7   (Bitboard b) { return b << 49; }

// right (east)
    inline Bitboard m_r1    (Bitboard b) { return b >>  1; }
    inline Bitboard m_r2    (Bitboard b) { return b >>  2; }
    inline Bitboard m_r3    (Bitboard b) { return b >>  3; }
    inline Bitboard m_r4    (Bitboard b) { return b >>  4; }
    inline Bitboard m_r5    (Bitboard b) { return b >>  5; }
    inline Bitboard m_r6    (Bitboard b) { return b >>  6; }
    inline Bitboard m_r7    (Bitboard b) { return b >>  7; }

// down‐right (south‐east)
    inline Bitboard m_dr1   (Bitboard b) { return b >>  9; }
    inline Bitboard m_dr2   (Bitboard b) { return b >> 18; }
    inline Bitboard m_dr3   (Bitboard b) { return b >> 27; }
    inline Bitboard m_dr4   (Bitboard b) { return b >> 36; }
    inline Bitboard m_dr5   (Bitboard b) { return b >> 45; }
    inline Bitboard m_dr6   (Bitboard b) { return b >> 54; }
    inline Bitboard m_dr7   (Bitboard b) { return b >> 63; }

// down (south)
    inline Bitboard m_down1 (Bitboard b) { return b >>  8; }
    inline Bitboard m_down2 (Bitboard b) { return b >> 16; }
    inline Bitboard m_down3 (Bitboard b) { return b >> 24; }
    inline Bitboard m_down4 (Bitboard b) { return b >> 32; }
    inline Bitboard m_down5 (Bitboard b) { return b >> 40; }
    inline Bitboard m_down6 (Bitboard b) { return b >> 48; }
    inline Bitboard m_down7 (Bitboard b) { return b >> 56; }

// down‐left (south‐west)
    inline Bitboard m_dl1   (Bitboard b) { return b >>  7; }
    inline Bitboard m_dl2   (Bitboard b) { return b >> 14; }
    inline Bitboard m_dl3   (Bitboard b) { return b >> 21; }
    inline Bitboard m_dl4   (Bitboard b) { return b >> 28; }
    inline Bitboard m_dl5   (Bitboard b) { return b >> 35; }
    inline Bitboard m_dl6   (Bitboard b) { return b >> 42; }
    inline Bitboard m_dl7   (Bitboard b) { return b >> 49; }

// left (west)
    inline Bitboard m_l1    (Bitboard b) { return b <<  1; }
    inline Bitboard m_l2    (Bitboard b) { return b <<  2; }
    inline Bitboard m_l3    (Bitboard b) { return b <<  3; }
    inline Bitboard m_l4    (Bitboard b) { return b <<  4; }
    inline Bitboard m_l5    (Bitboard b) { return b <<  5; }
    inline Bitboard m_l6    (Bitboard b) { return b <<  6; }
    inline Bitboard m_l7    (Bitboard b) { return b <<  7; }

// up‐left (north‐west)
    inline Bitboard m_ul1   (Bitboard b) { return b <<  9; }
    inline Bitboard m_ul2   (Bitboard b) { return b << 18; }
    inline Bitboard m_ul3   (Bitboard b) { return b << 27; }
    inline Bitboard m_ul4   (Bitboard b) { return b << 36; }
    inline Bitboard m_ul5   (Bitboard b) { return b << 45; }
    inline Bitboard m_ul6   (Bitboard b) { return b << 54; }
    inline Bitboard m_ul7   (Bitboard b) { return b << 63; }

//------------------------------------------------------------------------------
// 2) Knight jumps (56–63)
//------------------------------------------------------------------------------
    inline Bitboard m_kn15  (Bitboard b) { return b << 15; }
    inline Bitboard m_kn6   (Bitboard b) { return b <<  6; }
    inline Bitboard m_knm10 (Bitboard b) { return b >> 10; }
    inline Bitboard m_knm17 (Bitboard b) { return b >> 17; }
    inline Bitboard m_knm15 (Bitboard b) { return b >> 15; }
    inline Bitboard m_knm6  (Bitboard b) { return b >>  6; }
    inline Bitboard m_kn10  (Bitboard b) { return b << 10; }
    inline Bitboard m_kn17  (Bitboard b) { return b << 17; }

//------------------------------------------------------------------------------
// 3) Under‑promotions (64–72), same shifts as UL, U, UR
//------------------------------------------------------------------------------
    inline Bitboard m_upromUL(Bitboard b) { return b <<  9; }
    inline Bitboard m_upromU (Bitboard b) { return b <<  8; }
    inline Bitboard m_upromUR(Bitboard b) { return b <<  7; }
    inline Bitboard m_upromBL(Bitboard b) { return b <<  9; }
    inline Bitboard m_upromB (Bitboard b) { return b <<  8; }
    inline Bitboard m_upromBR(Bitboard b) { return b <<  7; }
    inline Bitboard m_upromCL(Bitboard b) { return b <<  9; }
    inline Bitboard m_upromC (Bitboard b) { return b <<  8; }
    inline Bitboard m_upromCR(Bitboard b) { return b <<  7; }

//------------------------------------------------------------------------------
// 4) MovementFunctions array (index → function)
//------------------------------------------------------------------------------
    inline constexpr std::array<MoveFunc,73> MovementFunctions = {{
        //  0– 6: up1…up7
        m_up1,   m_up2,   m_up3,   m_up4,   m_up5,   m_up6,   m_up7,
        //  7–13: ur1…ur7
        m_ur1,   m_ur2,   m_ur3,   m_ur4,   m_ur5,   m_ur6,   m_ur7,
        // 14–20: r1…r7
        m_r1,    m_r2,    m_r3,    m_r4,    m_r5,    m_r6,    m_r7,
        // 21–27: dr1…dr7
        m_dr1,   m_dr2,   m_dr3,   m_dr4,   m_dr5,   m_dr6,   m_dr7,
        // 28–34: down1…down7
        m_down1, m_down2, m_down3, m_down4, m_down5, m_down6, m_down7,
        // 35–41: dl1…dl7
        m_dl1,   m_dl2,   m_dl3,   m_dl4,   m_dl5,   m_dl6,   m_dl7,
        // 42–48: l1…l7
        m_l1,    m_l2,    m_l3,    m_l4,    m_l5,    m_l6,    m_l7,
        // 49–55: ul1…ul7
        m_ul1,   m_ul2,   m_ul3,   m_ul4,   m_ul5,   m_ul6,   m_ul7,
        // 56–63: knight moves
        m_kn15,  m_kn6,   m_knm10, m_knm17, m_knm15, m_knm6,  m_kn10,  m_kn17,
        // 64–72: under‑promotions
        m_upromUL, m_upromU, m_upromUR,
        m_upromBL, m_upromB, m_upromBR,
        m_upromCL, m_upromC, m_upromCR
    }};

//------------------------------------------------------------------------------
// 5) shift → movement‐index map (Left_Shift_to_Movement_type), doesn't include cases +- 6 & 7
//------------------------------------------------------------------------------
    inline int shiftToMovementType(int shift) {
        switch(shift) {
            // up
            case   8: return  0;
            case  16: return  1;
            case  24: return  2;
            case  32: return  3;
            case  40: return  4;
            case  48: return  5;
            case  56: return  6;
                // up‑right
//            case   7: return  7;
            case  14: return  8;
            case  21: return  9;
            case  28: return 10;
            case  35: return 11;
            case  42: return 12;
            case  49: return 13;
                // right
            case  -1: return 14;
            case  -2: return 15;
            case  -3: return 16;
            case  -4: return 17;
            case  -5: return 18;
//            case  -6: return 19;
//            case  -7: return 20;
                // down‑right
            case  -9: return 21;
            case -18: return 22;
            case -27: return 23;
            case -36: return 24;
            case -45: return 25;
            case -54: return 26;
            case -63: return 27;
                // down
            case  -8: return 28;
            case -16: return 29;
            case -24: return 30;
            case -32: return 31;
            case -40: return 32;
            case -48: return 33;
            case -56: return 34;
                // down‑left
//            case  -7: return 35;
            case -14: return 36;
            case -21: return 37;
            case -28: return 38;
            case -35: return 39;
            case -42: return 40;
            case -49: return 41;
                // left
            case   1: return 42;
            case   2: return 43;
            case   3: return 44;
            case   4: return 45;
            case   5: return 46;
//            case   6: return 47;
//            case   7: return 48;
                // up‑left
            case   9: return 49;
            case  18: return 50;
            case  27: return 51;
            case  36: return 52;
            case  45: return 53;
            case  54: return 54;
            case  63: return 55;
                // knight
            case  15: return 56;
//            case   6: return 57;
            case -10: return 58;
            case -17: return 59;
            case -15: return 60;
//            case  -6: return 61;
            case  10: return 62;
            case  17: return 63;
            default: return -1;
        }
    }

} // namespace Chess

#endif // MOVEMENT_HPP
