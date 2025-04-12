#include <iostream>
#include <vector>
#include <cstdint>

// Include each piece header.
#include "../include/Pieces/Rooks.hpp"
#include "../include/Pieces/Bishops.hpp"
#include "../include/Pieces/Queens.hpp"
#include "../include/Pieces/Knights.hpp"
#include "../include/Pieces/Kings.hpp"
#include "../include/Pieces/Pawns.hpp"
#include "../include/Bitboard.hpp"

using namespace Chess;

// A helper to print bitboards in a more user-friendly way.
static void printMoves(const std::vector<uint64_t>& moves) {
    for (size_t i = 0; i < moves.size(); ++i) {
        std::cout << "Move " << i + 1 << ":\n";
        Bitboard::print(moves[i]);
        std::cout << "\n";
    }
}

/**
 * Test rooks on an empty board from a known square (e.g. A1).
 * Then, test with an enemy piece in the path.
 */
void testRooks() {
    std::cout << "=== TEST Rooks ===\n";

    // 1) White Rook on A1
    uint64_t a1 = 0x0000000000000001ULL; // (1 << 0)
    uint64_t empty = 0xffffffffffffffffULL;
    uint64_t enemy = 0ULL;

    Rooks rook(WHITE_ROOK, a1);
    auto moves = rook.all_moves(empty, enemy);

    std::cout << "Rook at A1, no enemies:\n";
    printMoves(moves);

    // 2) White Rook on A1 with an enemy at A3, and everything else empty
    // A3 is bit index 16 (row 2, col 0).
    // The bit at A3 is 1 << 16.
    uint64_t a3 = 0x0000000000010000ULL;
    enemy = a3;
    moves = rook.all_moves(empty & ~a3, enemy);

    std::cout << "Rook at A1, enemy at A3:\n";
    printMoves(moves);
}

/**
 * Test bishops in an empty board scenario, then with some blocking pieces.
 */
void testBishops() {
    std::cout << "=== TEST Bishops ===\n";

    // White Bishop on C1.
    // C1 is bit index 2.
    uint64_t c1 = 0x0000000000000004ULL;
    uint64_t empty = 0xffffffffffffffffULL;
    uint64_t enemy = 0ULL;

    Bishops bishop(WHITE_BISHOP, c1);
    auto moves = bishop.all_moves(empty, enemy);

    std::cout << "Bishop at C1, no enemies:\n";
    printMoves(moves);

    // Add an enemy at E3 (bit index 20).
    // E3 is row=2 col=4 => index= (2*8 + 4) = 20 => 1<<20
    uint64_t e3 = 1ULL << 20;
    enemy = e3;
    moves = bishop.all_moves(empty & ~e3, enemy);

    std::cout << "Bishop at C1, enemy at E3:\n";
    printMoves(moves);
}

/**
 * Test queens from default position, then from a custom position.
 */
void testQueens() {
    std::cout << "=== TEST Queens ===\n";

    // White Queen default: 0x0000000000000010 (bit index = 4).
    // But let's place a queen on D1.
    // D1 is bit index 3 => (1 << 3).
    uint64_t d1 = 1ULL << 3;
    uint64_t empty = 0xffffffffffffffffULL;
    uint64_t enemy = 0ULL;

    Queens queen(WHITE_QUEEN, d1);
    auto moves = queen.all_moves(empty, enemy);

    std::cout << "Queen at D1, no enemies:\n";
    printMoves(moves);

    // Add an enemy at D3 (index=19).
    uint64_t d3 = 1ULL << 19;
    enemy = d3;
    moves = queen.all_moves(empty & ~d3, enemy);

    std::cout << "Queen at D1, enemy at D3:\n";
    printMoves(moves);
}

/**
 * Test knights from a known square.
 */
void testKnights() {
    std::cout << "=== TEST Knights ===\n";

    // White Knight on B1 => index=1 => (1 << 1).
    uint64_t b1 = 1ULL << 1;
    uint64_t empty = 0xffffffffffffffffULL;
    uint64_t enemy = 0ULL;

    Knights knight(WHITE_KNIGHT, b1);
    auto moves = knight.all_moves(empty, enemy);

    std::cout << "Knight at B1, no enemies:\n";
    printMoves(moves);

    // Place an enemy at C3 => row=2, col=2 => index=18 => (1 << 18).
    uint64_t c3 = 1ULL << 18;
    enemy = c3;
    moves = knight.all_moves(empty & ~c3, enemy);

    std::cout << "Knight at B1, enemy at C3:\n";
    printMoves(moves);
}

/**
 * Test kings from a known square.
 */
void testKings() {
    std::cout << "=== TEST Kings ===\n";

    // White King on E1 => index=4 => (1 << 4).
    uint64_t e1 = 1ULL << 4;
    uint64_t empty = 0xffffffffffffffffULL;
    uint64_t enemy = 0ULL;

    Kings king(WHITE_KING, e1);
    auto moves = king.all_moves(empty, enemy);

    std::cout << "King at E1, no enemies:\n";
    printMoves(moves);

    // Add an enemy at E2 => index=12 => (1 << 12).
    uint64_t e2 = 1ULL << 12;
    enemy = e2;
    moves = king.all_moves(empty & ~e2, enemy);

    std::cout << "King at E1, enemy at E2:\n";
    printMoves(moves);
}

/**
 * Test pawns in a known scenario.
 */
void testPawns() {
    std::cout << "=== TEST Pawns ===\n";

    // White Pawn on second rank by default => 0x000000000000ff00
    // We'll do a single white pawn in front of E2 => E2 => index=12 => 1ULL<<12
    // But let's just rely on the constructor's default for WHITE_PAWN
    Pawns whitePawns(WHITE_PAWN, 0ULL); // force it to pick default
    uint64_t empty = 0xffffffffffffffffULL;
    uint64_t enemy = 0ULL;

    // This means we have all 8 white pawns on rank 2. Let's see the single and double moves
    auto moves = whitePawns.all_moves(empty, enemy);
    std::cout << "White Pawns (default) all moves, no enemies:\n";
    std::cout << "Should see single and double moves from the second rank.\n";
    printMoves(moves);

    // Place an enemy in front of the E2 pawn => E3 => index=20 => (1ULL << 20)
    // Also place an enemy diagonally => D3 => index=19 => (1ULL << 19)
    // So that we can see captures.
    uint64_t e3 = 1ULL << 20;
    uint64_t d3 = 1ULL << 19;
    enemy = (e3 | d3);

    // For empty squares, remove e3 and d3 from "empty"
    auto adjustedEmpty = empty & ~enemy;

    moves = whitePawns.all_moves(adjustedEmpty, enemy);
    std::cout << "White Pawns (default) with enemies at D3 and E3:\n";
    printMoves(moves);
}

int testPieces() {
    testRooks();
    std::cout << "-----------------------------------\n";

    testBishops();
    std::cout << "-----------------------------------\n";

    testQueens();
    std::cout << "-----------------------------------\n";

    testKnights();
    std::cout << "-----------------------------------\n";

    testKings();
    std::cout << "-----------------------------------\n";

    testPawns();
    std::cout << "-----------------------------------\n";

    std::cout << "All piece tests finished.\n";
    return 0;
}
