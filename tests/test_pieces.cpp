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

static void printBoards(const Bitboard& piece, uint64_t enemy, uint64_t empty, const std::string& msg = "") {
    std::cout << msg << "\n";
    Bitboard::print(piece.board, "Piece board");
    Bitboard::print(enemy,      "Enemy board");
    // optional if you want to see the empty squares too:
    // Bitboard::print(empty,   "Empty board");
    std::cout << std::endl;
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

    // Print boards (piece, enemy, empty) before generating moves.
    printBoards(rook, enemy, empty, "Rook at A1, no enemies");

    auto moves = rook.all_moves(empty, enemy);
    std::cout << "Moves:\n";
    printMoves(moves);

    // 2) White Rook on A1 with an enemy at A3 => bit index 16
    uint64_t a3 = 0x0000000000010000ULL;
    enemy = a3;
    auto adjustedEmpty = empty & ~enemy;

    printBoards(rook, enemy, adjustedEmpty, "Rook at A1, enemy at A3");

    moves = rook.all_moves(adjustedEmpty, enemy);
    std::cout << "Moves:\n";
    printMoves(moves);
}

/**
 * Test bishops in an empty board scenario, then with some blocking pieces.
 */
void testBishops() {
    std::cout << "=== TEST Bishops ===\n";

    // White Bishop on C1 => bit index 2 => (1<<2)
    uint64_t c1 = 0x0000000000000004ULL;
    uint64_t empty = 0xffffffffffffffffULL;
    uint64_t enemy = 0ULL;

    Bishops bishop(WHITE_BISHOP, c1);

    printBoards(bishop, enemy, empty, "Bishop at C1, no enemies");
    auto moves = bishop.all_moves(empty, enemy);
    std::cout << "Moves:\n";
    printMoves(moves);

    // Add an enemy at E3 => bit index=20 => (1 << 20).
    uint64_t e3 = 1ULL << 20;
    enemy = e3;
    auto adjustedEmpty = empty & ~enemy;

    printBoards(bishop, enemy, adjustedEmpty, "Bishop at C1, enemy at E3");
    moves = bishop.all_moves(adjustedEmpty, enemy);
    std::cout << "Moves:\n";
    printMoves(moves);
}

/**
 * Test queens from a known position.
 */
void testQueens() {
    std::cout << "=== TEST Queens ===\n";

    // White Queen on D1 => bit index=3 => (1 << 3).
    uint64_t d1 = 1ULL << 3;
    uint64_t empty = 0xffffffffffffffffULL;
    uint64_t enemy = 0ULL;

    Queens queen(WHITE_QUEEN, d1);

    printBoards(queen, enemy, empty, "Queen at D1, no enemies");
    auto moves = queen.all_moves(empty, enemy);
    std::cout << "Moves:\n";
    printMoves(moves);

    // Add an enemy at D3 => index=19 => (1 << 19).
    uint64_t d3 = 1ULL << 19;
    enemy = d3;
    auto adjustedEmpty = empty & ~enemy;

    printBoards(queen, enemy, adjustedEmpty, "Queen at D1, enemy at D3");
    moves = queen.all_moves(adjustedEmpty, enemy);
    std::cout << "Moves:\n";
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

    printBoards(knight, enemy, empty, "Knight at B1, no enemies");
    auto moves = knight.all_moves(empty, enemy);
    std::cout << "Moves:\n";
    printMoves(moves);

    // Place an enemy at C3 => index=18 => (1<<18).
    uint64_t c3 = 1ULL << 18;
    enemy = c3;
    auto adjustedEmpty = empty & ~enemy;

    printBoards(knight, enemy, adjustedEmpty, "Knight at B1, enemy at C3");
    moves = knight.all_moves(adjustedEmpty, enemy);
    std::cout << "Moves:\n";
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

    printBoards(king, enemy, empty, "King at E1, no enemies");
    auto moves = king.all_moves(empty, enemy);
    std::cout << "Moves:\n";
    printMoves(moves);

    // Add an enemy at E2 => index=12 => (1 << 12).
    uint64_t e2 = 1ULL << 12;
    enemy = e2;
    auto adjustedEmpty = empty & ~enemy;

    printBoards(king, enemy, adjustedEmpty, "King at E1, enemy at E2");
    moves = king.all_moves(adjustedEmpty, enemy);
    std::cout << "Moves:\n";
    printMoves(moves);
}

/**
 * Test pawns in a known scenario.
 */
void testPawns() {
    std::cout << "=== TEST Pawns ===\n";

    // White Pawn default => second rank => 0x000000000000ff00
    // We'll use the default constructor to place them there.
    Pawns whitePawns(WHITE_PAWN, 0ULL);
    uint64_t empty = 0xffffffffffffffffULL;
    uint64_t enemy = 0ULL;

    printBoards(whitePawns, enemy, empty, "White Pawns default, no enemies");
    auto moves = whitePawns.all_moves(empty, enemy);
    std::cout << "Moves:\n";
    printMoves(moves);

    // Place enemies at D3 => index=19, E3 => index=20
    uint64_t d3 = 1ULL << 19;
    uint64_t e3 = 1ULL << 20;
    enemy = (d3 | e3);
    auto adjustedEmpty = empty & ~enemy;

    printBoards(whitePawns, enemy, adjustedEmpty, "White Pawns default, enemies at D3 / E3");
    moves = whitePawns.all_moves(adjustedEmpty, enemy);
    std::cout << "Moves:\n";
    printMoves(moves);
}

void testPieces() {
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
}
