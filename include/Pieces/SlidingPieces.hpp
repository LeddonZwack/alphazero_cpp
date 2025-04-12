#ifndef SLIDING_PIECES_HPP
#define SLIDING_PIECES_HPP

#include "Pieces.hpp"
#include "Bitboard.hpp"
#include <cmath>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>

namespace Chess {

    class SlidingPieces : public Pieces {
    public:
        explicit SlidingPieces(PieceType t, uint64_t b = 0ULL)
                : Pieces(t, b) {}

        // MAIN FUNCTIONS

        // Returns the union of horizontal moves from a given bitboard position.
        // Parameters:
        //   b          - the bitboard of the piece
        //   empty      - bitboard of empty squares (default: all squares free)
        //   enemy      - bitboard of enemy pieces (default: all squares occupied)
        static inline uint64_t getHorizontalMoves(uint64_t b,
                                                  uint64_t empty = 0xffffffffffffffffULL,
                                                  uint64_t enemy = 0xffffffffffffffffULL) {
            uint64_t lray = getLeftHorizontalRay(b, empty, enemy);
            uint64_t rray = getRightHorizontalRay(b, empty, enemy);
            return lray | rray;
        }

        // Returns the union of vertical moves by rotating the bitboard via a diagonal flip.
        static inline uint64_t getVerticalMoves(uint64_t b,
                                                uint64_t empty = 0xffffffffffffffffULL,
                                                uint64_t enemy = 0xffffffffffffffffULL) {
            uint64_t flippedB = flipDiagonal(b);
            uint64_t flippedEmpty = flipDiagonal(empty);
            uint64_t flippedEnemy = flipDiagonal(enemy);
            uint64_t lray = getLeftHorizontalRay(flippedB, flippedEmpty, flippedEnemy);
            uint64_t rray = getRightHorizontalRay(flippedB, flippedEmpty, flippedEnemy);
            return flipDiagonal(lray | rray);
        }

        // Returns diagonal moves from bitboard b, using a coordinate‐based approach.
        static inline uint64_t getDiagonalMoves(uint64_t b,
                                                uint64_t empty = 0xffffffffffffffffULL,
                                                uint64_t enemy = 0xffffffffffffffffULL) {
            int index = get_index(b);
            uint64_t toRet = 0;
            if (index == 99) return toRet;
            // Use string reversals (via std::string) similar to Python slicing.
            std::string enemySub = Bitboard::toString(enemy);
            std::string emptySub = Bitboard::toString(empty);
            std::reverse(enemySub.begin(), enemySub.end());
            std::reverse(emptySub.begin(), emptySub.end());
            // For each diagonal direction, iterate until a blocker is encountered.
            auto processDirection = [&](int dx, int dy) {
                auto [x, y] = getCoors(index);
                x += dx;
                y += dy;
                while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                    int pos = getIndexFromCoors(x, y);
                    // If square is empty and not enemy, break.
                    if (emptySub[pos] == '0' && enemySub[pos] == '0')
                        break;
                    toRet |= (1ULL << pos);
                    if (enemySub[pos] == '1')
                        break;
                    x += dx;
                    y += dy;
                }
            };

            // Process the four diagonal directions:
            processDirection( 1,  1); // top left (in bitboard coordinates, increasing index)
            processDirection(-1,  1); // top right
            processDirection(-1, -1); // bottom right
            processDirection( 1, -1); // bottom left

            return toRet;
        }

        // Returns all rook moves for a composite bitboard (allowing multiple rooks)
        // as a vector of resulting bitboards.
        static inline std::vector<uint64_t> allRookMoves(uint64_t b,
                                                         uint64_t empty = 0xffffffffffffffffULL,
                                                         uint64_t enemy = 0xffffffffffffffffULL) {
            std::vector<uint64_t> moves;
            uint64_t rookBB = b;
            std::vector<uint64_t> isolated;
            // Isolate each rook: repeatedly extract LSB.
            while (rookBB) {
                uint64_t lsb = Bitboard::getLSB(rookBB);
                isolated.push_back(lsb);
                rookBB = Bitboard::removeLSB(rookBB, lsb);
            }
            for (auto rook : isolated) {
                uint64_t rookComplement = Bitboard::complement(rook);
                uint64_t vertical = getVerticalMoves(rook, empty, enemy & rookComplement);
                uint64_t horizontal = getHorizontalMoves(rook, empty, enemy & rookComplement);
                uint64_t movesBitboard = vertical | horizontal;
                uint64_t removedRook = b & Bitboard::complement(rook);
                while (movesBitboard) {
                    uint64_t moveLSB = Bitboard::getLSB(movesBitboard);
                    moves.push_back(removedRook | moveLSB);
                    movesBitboard = Bitboard::removeLSB(movesBitboard, moveLSB);
                }
            }
            return moves;
        }

        // Returns all bishop moves (diagonals) for a composite bitboard.
        static inline std::vector<uint64_t> allBishopMoves(uint64_t b,
                                                           uint64_t empty = 0xffffffffffffffffULL,
                                                           uint64_t enemy = 0xffffffffffffffffULL) {
            std::vector<uint64_t> moves;
            uint64_t bishopBB = b;
            std::vector<uint64_t> isolated;
            while (bishopBB) {
                uint64_t lsb = Bitboard::getLSB(bishopBB);
                isolated.push_back(lsb);
                bishopBB = Bitboard::removeLSB(bishopBB, lsb);
            }
            for (auto bishop : isolated) {
                uint64_t movesBitboard = getDiagonalMoves(bishop, empty, enemy);
                uint64_t removedBishop = b & Bitboard::complement(bishop);
                while (movesBitboard) {
                    uint64_t moveLSB = Bitboard::getLSB(movesBitboard);
                    moves.push_back(removedBishop | moveLSB);
                    movesBitboard = Bitboard::removeLSB(movesBitboard, moveLSB);
                }
            }
            return moves;
        }

        // HELPER FUNCTIONS

        // Returns the index (0-63) of the least-significant bit; returns 99 if input is zero.
        static inline int get_index(uint64_t b) {
            if (b == 0) return 99;
            return static_cast<int>(std::log2(Bitboard::getLSB(b)));
        }

        // Given an index, returns (x,y) coordinates (x = column, y = row)
        static inline std::pair<int, int> getCoors(int index) {
            return { index % 8, index / 8 };
        }

        // Returns the linear index from coordinates (x,y)
        static inline int getIndexFromCoors(int x, int y) {
            return y * 8 + x;
        }

        // Returns the left horizontal ray from bitboard b.
        static inline uint64_t getLeftHorizontalRay(uint64_t b, uint64_t empty, uint64_t enemy, const std::string& debugMsg = "") {
            int rookIndex = get_index(b);
            if (rookIndex == 99) return 0;
            int distFromRightWall = rookIndex % 8;
            int distFromLeftWall = 7 - distFromRightWall;
            uint64_t enemyShifted = enemy >> rookIndex;
            int distFromEnemy = get_index(Bitboard::getLSB(enemyShifted)) - 1;
            uint64_t blockers = Bitboard::complement(empty) & Bitboard::complement(b) & Bitboard::complement(enemy);
            uint64_t blockersShifted = blockers >> rookIndex;
            int distFromBlocker = get_index(Bitboard::getLSB(blockersShifted)) - 1;
            int limiting = std::min(distFromLeftWall, std::min(distFromEnemy, distFromBlocker));
            if (limiting == distFromEnemy && limiting >= 0 && distFromBlocker > limiting && distFromLeftWall > 0 && distFromLeftWall != distFromEnemy)
                limiting += 1;
            if (limiting == -1) limiting = 0;
            return ((1ULL << limiting) - 1) << (rookIndex + 1);
        }

        // Returns the right horizontal ray by reversing the bitboard.
        static inline uint64_t getRightHorizontalRay(uint64_t b, uint64_t empty, uint64_t enemy, const std::string& debugMsg = "") {
            uint64_t revB = Bitboard::reverse(b);
            uint64_t revEnemy = Bitboard::reverse(enemy);
            uint64_t revEmpty = Bitboard::reverse(empty);
            uint64_t revRay = getLeftHorizontalRay(revB, revEmpty, revEnemy, debugMsg);
            return Bitboard::reverse(revRay);
        }

        // Flips the diagonal by performing a series of bit manipulations.
        static inline uint64_t flipDiagonal(uint64_t bb) {
            uint64_t t = 0;
            const uint64_t k1 = 0x5500550055005500ULL;
            const uint64_t k2 = 0x3333000033330000ULL;
            const uint64_t k4 = 0x0f0f0f0f00000000ULL;
            t = k4 & (bb ^ (bb << 28));
            bb ^= t ^ (t >> 28);
            t = k2 & (bb ^ (bb << 14));
            bb ^= t ^ (t >> 14);
            t = k1 & (bb ^ (bb << 7));
            bb ^= t ^ (t >> 7);
            return bb;
        }
    };

} // namespace Chess

#endif // SLIDING_PIECES_HPP
