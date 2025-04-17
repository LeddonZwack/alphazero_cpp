#ifndef MOVE_MAPPING_HPP
#define MOVE_MAPPING_HPP

#include "bitboard/piece_type.hpp"
#include <cstdint>
#include <array>

// We assume PieceType values are defined as:
// 0: WHITE_PAWN, 1: WHITE_KNIGHT, 2: WHITE_BISHOP, 3: WHITE_ROOK, 4: WHITE_QUEEN, 5: WHITE_KING,
// 6: BLACK_PAWN, 7: BLACK_KNIGHT, 8: BLACK_BISHOP, 9: BLACK_ROOK, 10: BLACK_QUEEN, 11: BLACK_KING

namespace MoveMapping {

// Total number of movement types (0 to 72)
    static constexpr int MOVEMENT_TYPE_COUNT = 73;

    constexpr int offset = 63;

// This array maps movement type index to its associated shift value (in bits).
// Positive numbers indicate a left shift, negative a right shift.
    static constexpr std::array<int8_t, MOVEMENT_TYPE_COUNT> moveTypeToShift = {
            // 0–6: up moves (<<8, <<16, …, <<56)
            8,  16, 24, 32, 40, 48, 56,
            // 7–13: up-right moves (<<7, <<14, …, <<49)
            7, 14, 21, 28, 35, 42, 49,
            // 14–20: right moves (>>1, >>2, …, >>7)
            -1, -2, -3, -4, -5, -6, -7,  // note: -6 and -7 below are ambiguous
            // 21–27: down-right moves (>>9, >>18, …, >>63)
            -9, -18, -27, -36, -45, -54, -63,
            // 28–34: down moves (>>8, >>16, …, >>56)
            -8, -16, -24, -32, -40, -48, -56,
            // 35–41: down-left moves (>>7, >>14, …, >>49)
            -7, -14, -21, -28, -35, -42, -49,  // again, -7 is ambiguous
            // 42–48: left moves (<<1, <<2, …, <<7)
            1,  2,  3,  4,  5,  6,  7,         // here +6 and +7 are ambiguous
            // 49–55: up-left moves (<<9, <<18, …, <<63)
            9, 18, 27, 36, 45, 54, 63,
            // 56–63: knight moves (unique)
            15, 6, -10, -17, -15, -6, 10, 17,
            // 64–72: underpromotions (using similar shifts to queen moves)
            9, 8, 7, 9, 8, 7, 9, 8, 7
    };

//
// Reverse lookup table: for a given shift value from -63 to +63,
// we want to quickly obtain the corresponding movement type, if unambiguous.
// The table is indexed by (shift + 63) so that index 0 corresponds to shift -63,
// and index 126 corresponds to shift +63.
//
// For ambiguous shifts (i.e. exactly +7, -7, +6, or -6 for non-knights), we mark them with AMBIGUOUS.
    static constexpr int AMBIGUOUS = -1;
    static constexpr std::array<int, 127> reverseMap = []() constexpr -> std::array<int, 127> {
        std::array<int, 127> arr = {};
        // Initialize all entries to -1.
        for (int i = 0; i < 127; ++i)
            arr[i] = AMBIGUOUS;
        // Fill in the unambiguous moves from moveTypeToShift.
        // Up moves:
        arr[8   + offset] = 0;
        arr[16  + offset] = 1;
        arr[24  + offset] = 2;
        arr[32  + offset] = 3;
        arr[40  + offset] = 4;
        arr[48  + offset] = 5;
        arr[56  + offset] = 6;
        // Up-right: shift 7 is ambiguous.
        arr[7  + offset] = AMBIGUOUS;
        arr[14  + offset] = 8;
        arr[21  + offset] = 9;
        arr[28  + offset] = 10;
        arr[35  + offset] = 11;
        arr[42  + offset] = 12;
        arr[49  + offset] = 13;
        // Right: -6 and -7 ambiguous.
        arr[-1  + offset] = 14;
        arr[-2  + offset] = 15;
        arr[-3  + offset] = 16;
        arr[-4  + offset] = 17;
        arr[-5  + offset] = 18;
        arr[-6  + offset] = AMBIGUOUS;
        arr[-7  + offset] = AMBIGUOUS;
        // Down-right:
        arr[-9  + offset] = 21;
        arr[-18 + offset] = 22;
        arr[-27 + offset] = 23;
        arr[-36 + offset] = 24;
        arr[-45 + offset] = 25;
        arr[-54 + offset] = 26;
        arr[-63 + offset] = 27;
        // Down:
        arr[-8  + offset] = 28;
        arr[-16 + offset] = 29;
        arr[-24 + offset] = 30;
        arr[-32 + offset] = 31;
        arr[-40 + offset] = 32;
        arr[-48 + offset] = 33;
        arr[-56 + offset] = 34;
        // Down-left: shift -7 ambiguous;
        arr[-7  + offset] = AMBIGUOUS;
        arr[-14 + offset] = 36;
        arr[-21 + offset] = 37;
        arr[-28 + offset] = 38;
        arr[-35 + offset] = 39;
        arr[-42 + offset] = 40;
        arr[-49 + offset] = 41;
        // Left: +6 and +7 ambiguous.
        arr[+1  + offset] = 42;
        arr[+2  + offset] = 43;
        arr[+3  + offset] = 44;
        arr[+4  + offset] = 45;
        arr[+5  + offset] = 46;
        arr[+6  + offset] = AMBIGUOUS;
        arr[+7  + offset] = AMBIGUOUS;
        // Up-left:
        arr[+9  + offset] = 49;
        arr[+18 + offset] = 50;
        arr[+27 + offset] = 51;
        arr[+36 + offset] = 52;
        arr[+45 + offset] = 53;
        arr[+54 + offset] = 54;
        arr[+63 + offset] = 55;
        // Knight moves:
        arr[+15 + offset] = 56;
        arr[+6  + offset] = AMBIGUOUS;
        arr[-10 + offset] = 58;
        arr[-17 + offset] = 59;
        arr[-15 + offset] = 60;
        arr[-6  + offset] = AMBIGUOUS;
        arr[+10 + offset] = 62;
        arr[+17 + offset] = 63;

        return arr;
    }();

//
// getMovementType:
//   Given a shift offset (the difference in bit positions between a destination and source, which can be negative)
//   and the from-square index as well as the piece type, returns the correct movement type index.
//
// For ambiguous shifts (i.e. ±7 and ±6 in sliding and knight moves), we resolve them via the from square’s file (and piece type if applicable).
//
    inline int getMovementType(int shift, int fromSquare, int pieceType) {
        int index = shift + offset;
        int baseType = reverseMap[index];
        if (baseType != AMBIGUOUS)
            return baseType;
        // Handle ambiguous cases:
        int file = fromSquare % 8;
        // For shift == +7 (up-right or left move depending on context)
        if (shift == 7) {
            // For left moves: if from file == 0, then cannot move left normally—interpret as under left-bound move.
            return (file == 0) ? 48 : 7;
        }
        else if (shift == -7) {
            return (file == 7) ? 20 : 35;
        }
        else if (shift == 6 && pieceType != bb::WHITE_KNIGHT) { // assuming pieceType 1 is knight; adjust as needed
            return (file <= 1) ? 47 : 57;
        }
        else if (shift == -6 && pieceType != bb::WHITE_KNIGHT) {
            return (file >= 6) ? 19 : 61;
        }
        // If no match is found, return -1 as error.
        return -1;
    }

/// Apply a movement to a one–bit piece bitboard.
///   from_bb should have exactly one bit set;
///   moveType is an index in [0, MOVEMENT_TYPE_COUNT).
    inline uint64_t applyMovement(uint64_t from_bb, int moveType) {
        int8_t shift = moveTypeToShift[moveType];
        if (shift > 0)
            return from_bb << shift;
        else
            return from_bb >> (-shift);
    }

    // Get movement types for pawn promotions
    inline std::array<int, 3> getPromotionMovementTypes(int pieceType, uint64_t toBitboard, int shiftAmount) {
        constexpr uint64_t RANK_8_MASK = 0xff00000000000000ULL;

        if (pieceType != bb::WHITE_PAWN || (toBitboard & RANK_8_MASK) == 0)
            return {-1, -1, -1}; // Not a white pawn promotion

        switch (shiftAmount) {
            case 9:  return {64, 67, 70}; // Knight, Bishop, Rook promotions (right capture)
            case 8:  return {65, 68, 71}; // Knight, Bishop, Rook promotions (forward)
            case 7:  return {66, 69, 72}; // Knight, Bishop, Rook promotions (left capture)
            default: return {-1, -1, -1}; // Not a promotion-related shift
        }
    }

} // namespace MoveMapping

#endif // MOVE_MAPPING_HPP
