#ifndef PIECES_HPP
#define PIECES_HPP

#include <cstdint>
#include <string>
#include <iostream>
#include "Bitboard.hpp"  // Assumes include/ is on your include path

namespace Chess {

// Colors
    enum Color : int {
        WHITE =  1,
        BLACK = -1
    };

// Piece types
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
        BLACK_KING   = 11
    };

/**
 * Base class for all piece bitboards.
 * Inherits the raw 64‑bit board from Bitboard.
 */
    class Pieces : public Bitboard {
    public:
        PieceType type;
        std::string symbol;
        bool mark;

        // Construct with a type and optional initial bitboard
        explicit Pieces(PieceType t, uint64_t b = 0ULL)
                : Bitboard(b), type(t), symbol(""), mark(false) {}

        virtual ~Pieces() = default;

        // Default move() stub; override in derived classes
        virtual void move() const {
            std::cerr << "Error: use overridden piece‑specific move().\n";
        }

        // Print the name of a piece type
        static void printPieceType(PieceType pt);
    };

} // namespace Chess

#endif // PIECES_HPP
