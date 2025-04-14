// tests/test_bitboard.cpp

#include "bitboard/bitboard_utils.hpp"
#include "bitboard/movegen_simple.hpp"
#include "bitboard/movegen_sliding.hpp"
#include "bitboard/piece_type.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace bb;
using namespace bb_utils;

static int tests_run = 0;
static int tests_failed = 0;

// Helper to print a bitboard with a label.
static void debug_print_bitboard(uint64_t b, const std::string &label) {
    std::cout << label << "\n";
    print(b, label);
}

// Custom assertion macros that also print debug info if the assertion fails.
#define ASSERT_EQ(a,b) do { \
    tests_run++; \
    if ((a) != (b)) { \
        std::cerr << __FILE__ << ":" << __LINE__ << " Assertion failed: " << #a << " != " << #b \
                  << " (" << (a) << " vs " << (b) << ")\n"; \
        tests_failed++; \
    } \
} while(0)

#define ASSERT_TRUE(cond) do { \
    tests_run++; \
    if (!(cond)) { \
        std::cerr << __FILE__ << ":" << __LINE__ << " Assertion failed: " << #cond << "\n"; \
        tests_failed++; \
    } \
} while(0)

// For vector-of-bitboards tests, if sizes mismatch, print all boards.
static void debug_print_moves(const std::vector<uint64_t>& moves, const std::string &testName) {
    std::cout << "Debug info for " << testName << ":\n";
    std::cout << "Number of moves: " << moves.size() << "\n";
    for (size_t i = 0; i < moves.size(); ++i) {
        std::ostringstream oss;
        oss << testName << " move " << i;
        debug_print_bitboard(moves[i], oss.str());
    }
}

void test_knight_moves_center() {
    // Knight on d4 (rank3,file3, idx = 27)
    uint64_t knights = 1ULL << 27;
    uint64_t empty   = complement(knights);
    uint64_t enemy   = 0;
    auto moves = generate_knight_moves(knights, empty, enemy);

    std::vector<int> expected = {10,12,17,21,33,37,42,44};
    if(moves.size() != expected.size()){
        std::cerr << "test_knight_moves_center: Expected " << expected.size()
                  << " moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "Knight Center");
    }
    ASSERT_EQ(moves.size(), expected.size());

    std::vector<int> actual;
    for (auto bb : moves) {
        actual.push_back(ctz(bb));
    }
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    for (size_t i = 0; i < expected.size(); ++i) {
        if(actual[i] != expected[i]){
            std::cerr << "test_knight_moves_center: Move " << i
                      << " expected index " << expected[i]
                      << " but got " << actual[i] << "\n";
        }
        ASSERT_EQ(actual[i], expected[i]);
    }
}

void test_knight_moves_corner() {
    // Knight on a1 (idx = 0)
    uint64_t knights = 1ULL << 0;
    uint64_t empty   = complement(knights);
    uint64_t enemy   = 0;
    auto moves = generate_knight_moves(knights, empty, enemy);

    std::vector<int> expected = {10,17};
    if(moves.size() != expected.size()){
        std::cerr << "test_knight_moves_corner: Expected " << expected.size()
                  << " moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "Knight Corner");
    }
    ASSERT_EQ(moves.size(), expected.size());

    std::vector<int> actual;
    for (auto bb : moves)
        actual.push_back(ctz(bb));
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    for (size_t i = 0; i < expected.size(); ++i) {
        if(actual[i] != expected[i]){
            std::cerr << "test_knight_moves_corner: Move " << i
                      << " expected index " << expected[i]
                      << " but got " << actual[i] << "\n";
        }
        ASSERT_EQ(actual[i], expected[i]);
    }
}

void test_king_moves_center() {
    // King on d4 (idx = 27)
    uint64_t kings = 1ULL << 27;
    uint64_t empty = complement(kings);
    uint64_t enemy = 0;
    auto moves = generate_king_moves(kings, empty, enemy);

    std::vector<int> expected = {18,19,20,26,28,34,35,36};
    if(moves.size() != expected.size()){
        std::cerr << "test_king_moves_center: Expected " << expected.size()
                  << " moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "King Center");
    }
    ASSERT_EQ(moves.size(), expected.size());

    std::vector<int> actual;
    for (auto bb : moves)
        actual.push_back(ctz(bb));
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    for (size_t i = 0; i < expected.size(); ++i) {
        if(actual[i] != expected[i]){
            std::cerr << "test_king_moves_center: Move " << i
                      << " expected index " << expected[i]
                      << " but got " << actual[i] << "\n";
        }
        ASSERT_EQ(actual[i], expected[i]);
    }
}

void test_king_moves_corner() {
    // King on a1 (idx = 0)
    uint64_t kings = 1ULL << 0;
    uint64_t empty = complement(kings);
    uint64_t enemy = 0;
    auto moves = generate_king_moves(kings, empty, enemy);

    std::vector<int> expected = {1,8,9};
    if(moves.size() != expected.size()){
        std::cerr << "test_king_moves_corner: Expected " << expected.size()
                  << " moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "King Corner");
    }
    ASSERT_EQ(moves.size(), expected.size());

    std::vector<int> actual;
    for (auto bb : moves)
        actual.push_back(ctz(bb));
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    for (size_t i = 0; i < expected.size(); ++i) {
        if(actual[i] != expected[i]){
            std::cerr << "test_king_moves_corner: Move " << i
                      << " expected index " << expected[i]
                      << " but got " << actual[i] << "\n";
        }
        ASSERT_EQ(actual[i], expected[i]);
    }
}

void test_pawn_moves_white_pushes() {
    // White pawn on e2 (idx = 12)
    uint64_t pawns = 1ULL << 12;
    uint64_t empty = complement(pawns);
    uint64_t enemy = 0;
    auto moves = generate_pawn_moves(pawns, empty, enemy, WHITE_PAWN);

    std::vector<int> expected = {20,28}; // e3 and e4 (indices 20 and 28)
    if(moves.size() != expected.size()){
        std::cerr << "test_pawn_moves_white_pushes: Expected " << expected.size()
                  << " moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "White Pawn Pushes");
    }
    ASSERT_EQ(moves.size(), expected.size());

    std::vector<int> actual;
    for (auto bb : moves)
        actual.push_back(ctz(bb));
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    for (size_t i = 0; i < expected.size(); ++i) {
        if(actual[i] != expected[i]){
            std::cerr << "test_pawn_moves_white_pushes: Move " << i
                      << " expected index " << expected[i]
                      << " but got " << actual[i] << "\n";
            // Print pawn, empty board and the move
            debug_print_bitboard(pawns, "Original Pawn");
            debug_print_bitboard(empty, "Empty Board");
        }
        ASSERT_EQ(actual[i], expected[i]);
    }
}

void test_pawn_moves_white_captures() {
    // White pawn on e2, enemy pieces on d3 (19) and f3 (21)
    uint64_t pawns = 1ULL << 12;
    uint64_t enemy = (1ULL << 19) | (1ULL << 21);
    uint64_t empty = complement(pawns | enemy);

//    // Extra debugging
//    print(pawns, "White Pawns: ");
//    print(enemy, "Black Pawns: ");
//    print(empty, "Empty Squares: ");

    auto moves = generate_pawn_moves(pawns, empty, enemy, WHITE_PAWN);

    std::vector<int> expected = {19,21, 20, 28};
    if(moves.size() != expected.size()){
        std::cerr << "test_pawn_moves_white_captures: Expected " << expected.size()
                  << " moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "White Pawn Captures");
    }
    ASSERT_EQ(moves.size(), expected.size());

    std::vector<int> actual;
    for (auto bb : moves)
        actual.push_back(ctz(bb));
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    for (size_t i = 0; i < expected.size(); ++i) {
        if(actual[i] != expected[i]){
            std::cerr << "test_pawn_moves_white_captures: Move " << i
                      << " expected index " << expected[i]
                      << " but got " << actual[i] << "\n";
        }
        ASSERT_EQ(actual[i], expected[i]);
    }
}

void test_pawn_moves_black_pushes() {
    // Black pawn on e7 (idx = 52)
    uint64_t pawns = 1ULL << 52;
    uint64_t empty = complement(pawns);
    uint64_t enemy = 0;
    auto moves = generate_pawn_moves(pawns, empty, enemy, BLACK_PAWN);

    std::vector<int> expected = {36,44}; // e5 and e6 (indices 36 and 44)
    if(moves.size() != expected.size()){
        std::cerr << "test_pawn_moves_black_pushes: Expected " << expected.size()
                  << " moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "Black Pawn Pushes");
    }
    ASSERT_EQ(moves.size(), expected.size());

    std::vector<int> actual;
    for (auto bb : moves)
        actual.push_back(ctz(bb));
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    for (size_t i = 0; i < expected.size(); ++i) {
        if(actual[i] != expected[i]){
            std::cerr << "test_pawn_moves_black_pushes: Move " << i
                      << " expected index " << expected[i]
                      << " but got " << actual[i] << "\n";
            debug_print_bitboard(pawns, "Original Black Pawn");
            debug_print_bitboard(empty, "Empty Board");
        }
        ASSERT_EQ(actual[i], expected[i]);
    }
}

void test_pawn_moves_black_captures() {
    // Black pawn on e7, enemy pieces on d6 (43) and f6 (45)
    uint64_t pawns = 1ULL << 52;
    uint64_t enemy = (1ULL << 43) | (1ULL << 45);
    uint64_t empty = complement(pawns | enemy);

//    // Extra debugging
//    print(pawns, "Black Pawns: ");
//    print(enemy, "White Pawns: ");
//    print(empty, "Empty Squares: ");

    auto moves = generate_pawn_moves(pawns, empty, enemy, BLACK_PAWN);

    std::vector<int> expected = {43,45, 44, 36};
    if(moves.size() != expected.size()){
        std::cerr << "test_pawn_moves_black_captures: Expected " << expected.size()
                  << " moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "Black Pawn Captures");
    }
    ASSERT_EQ(moves.size(), expected.size());

    std::vector<int> actual;
    for (auto bb : moves)
        actual.push_back(ctz(bb));
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    for (size_t i = 0; i < expected.size(); ++i) {
        if(actual[i] != expected[i]){
            std::cerr << "test_pawn_moves_black_captures: Move " << i
                      << " expected index " << expected[i]
                      << " but got " << actual[i] << "\n";
        }
        ASSERT_EQ(actual[i], expected[i]);
    }
}

void test_rook_moves_empty() {
    // Rook on d4 (idx = 27), empty board
    uint64_t rooks = 1ULL << 27;
    uint64_t empty = complement(rooks);
    uint64_t enemy = 0;
    auto moves = generate_rook_moves(rooks, empty, enemy);

    std::vector<int> expected = {
            3,11,19,    // downward moves
            24,25,26,   // left
            28,29,30,31,// right
            35,43,51,59 // upward moves
    };
    if(moves.size() != expected.size()){
        std::cerr << "test_rook_moves_empty: Expected " << expected.size()
                  << " moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "Rook Moves");
    }
    ASSERT_EQ(moves.size(), expected.size());

    std::vector<int> actual;
    for (auto bb : moves)
        actual.push_back(ctz(bb));
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    for (size_t i = 0; i < expected.size(); ++i) {
        if(actual[i] != expected[i]){
            std::cerr << "test_rook_moves_empty: Move " << i
                      << " expected index " << expected[i]
                      << " but got " << actual[i] << "\n";
        }
        ASSERT_EQ(actual[i], expected[i]);
    }
}

void test_bishop_moves_empty() {
    // Bishop on d4 (idx = 27), empty board
    uint64_t bishops = 1ULL << 27;
    uint64_t empty   = complement(bishops);
    uint64_t enemy   = 0;
    auto moves = generate_bishop_moves(bishops, empty, enemy);

    std::vector<int> expected = {
            0,6,9,13,18,20,34,36,41,45,48,54,63
    };
    if(moves.size() != expected.size()){
        std::cerr << "test_bishop_moves_empty: Expected " << expected.size()
                  << " moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "Bishop Moves");
    }
    ASSERT_EQ(moves.size(), expected.size());

    std::vector<int> actual;
    for (auto bb : moves)
        actual.push_back(ctz(bb));
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    for (size_t i = 0; i < expected.size(); ++i) {
        if(actual[i] != expected[i]){
            std::cerr << "test_bishop_moves_empty: Move " << i
                      << " expected index " << expected[i]
                      << " but got " << actual[i] << "\n";
        }
        ASSERT_EQ(actual[i], expected[i]);
    }
}

void test_queen_moves_empty() {
    // Queen on d4 (idx = 27), empty board.
    // Expected: union of rook moves (14 moves) and bishop moves (13 moves) = 27 moves.
    uint64_t queens = 1ULL << 27;
    uint64_t empty  = complement(queens);
    uint64_t enemy  = 0;
    auto moves = generate_queen_moves(queens, empty, enemy);

    if(moves.size() != 27){
        std::cerr << "test_queen_moves_empty: Expected 27 moves, got " << moves.size() << "\n";
        debug_print_moves(moves, "Queen Moves");
    }
    ASSERT_EQ(moves.size(), 27u);

    // Sanity check: every move should be a single square move.
    for (auto bb : moves) {
        if(popcount(bb) != 1) {
            std::cerr << "test_queen_moves_empty: A generated move does not contain a single bit.\n";
            debug_print_bitboard(bb, "Faulty Queen move");
        }
        ASSERT_TRUE(popcount(bb) == 1);
        int idx = ctz(bb);
        ASSERT_TRUE(idx >= 0 && idx < 64);
    }
}

extern "C" void run_all_bitboard_tests() {
    test_knight_moves_center();
    test_knight_moves_corner();
    test_king_moves_center();
    test_king_moves_corner();
    test_pawn_moves_white_pushes();
    test_pawn_moves_white_captures();
    test_pawn_moves_black_pushes();
    test_pawn_moves_black_captures();
    test_rook_moves_empty();
    test_bishop_moves_empty();
    test_queen_moves_empty();

    std::cout << "\nTests run:    " << tests_run
              << "\nFailures:     " << tests_failed << "\n";
    if (tests_failed == 0) {
        std::cout << "ALL BITBOARD TESTS PASSED ✅\n";
    }
}
