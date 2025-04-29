#include "MoveGeneration.hpp"
#include "State.hpp"
#include "bitboard/movegen_simple.hpp"   // Contains bb::generate_knight_moves, etc.
#include "bitboard/movegen_sliding.hpp"  // Contains bb::generate_rook_moves, etc.
#include "bitboard/bitboard_utils.hpp"            // for bb_utils::ctz and pop_lsb
#include "StateTransition.hpp"           // Provides tempApplyActionToPieces
#include "MoveMapping.hpp"               // Provides getMovementType() and applyMovement()

#include <array>
#include <vector>
#include <cassert>

namespace MoveGeneration {

    constexpr char PIECE_CHAR[13] = {
            'P','N','B','R','Q','K',   // 0-5  white pieces
            'p','n','b','r','q','k',   // 6-11 black pieces
            '.'                        // 12   empty
    };

    constexpr uint64_t RANK_8_MASK = 0xff00000000000000ULL;

    /*  pieces[0] = WHITE_PAWN  … pieces[11] = BLACK_KING
    Bit 0 = a1, bit 63 = h8  */
    static void dbgPrintBoard(const std::array<uint64_t,12>& pieces,
                              std::ostream& os = std::cout)
    {
        char board[64];
        std::fill(std::begin(board), std::end(board), PIECE_CHAR[12]);   // fill with '.'

        for (int pt = 0; pt < 12; ++pt) {
            uint64_t bb = pieces[pt];
            while (bb) {
                uint64_t lsb = bb & -bb;
                #ifdef __GNUC__
                int idx = __builtin_ctzll(bb);          // 0-63
                #else
                int idx = std::countr_zero(bb);         // C++20 fallback
                #endif
                board[idx] = PIECE_CHAR[pt];
                bb &= bb - 1;                           // clear lsb
            }
        }

        os << "   a b c d e f g h\n";
        for (int rank = 7; rank >= 0; --rank) {
            os << rank + 1 << "  ";
            for (int file = 7; file >= 0; --file)
                os << board[rank * 8 + file] << ' ';
            os << '\n';
        }
        os << '\n';
    }

    // Compute the union of all pieces: empty squares = complement(white ∪ black),
    // enemyPieces = union of all black pieces (indices 6..11).
    std::pair<uint64_t, uint64_t> getImportantSquares(const std::array<uint64_t, 12> &pieces, int enemyColor) {
        uint64_t whitePieces = 0ULL, blackPieces = 0ULL;
        for (int pt = 0; pt < 6; ++pt)
            whitePieces |= pieces[pt];
        for (int pt = 6; pt < 12; ++pt)
            blackPieces |= pieces[pt];
        uint64_t occupied = whitePieces | blackPieces;
        uint64_t emptySquares = ~occupied;

        if (enemyColor == Chess::WHITE) return {emptySquares, whitePieces};
        else return {emptySquares, blackPieces};
    }

    // isInCheck operates solely on the pieces array.
    // It generates all moves for black pieces and checks if any destination covers white's king.
    bool isInCheck(const std::array<uint64_t, 12> &pieces) {
        // White king is at index WHITE_KING (which we assume equals 5).
        uint64_t whiteKing = pieces[bb::WHITE_KING];
        assert(whiteKing != 0);
        if (whiteKing == 0ULL){
            bb_utils::print(whiteKing, "Error with isInCheck.");
            return true;  // Should not happen.
        }

        // Getting black pieces moves so our enemy is white
        auto [emptySquares, enemyPieces] = getImportantSquares(pieces, Chess::WHITE);
        uint64_t attackMask = 0ULL;

        // For each black piece type, use the corresponding move generator.
        for (int pt = 6; pt < 12; ++pt) {
            // Call the move generator on the full bitboard for that piece type.
            const uint64_t pieceBB = pieces[pt];
            if (pieceBB == 0ULL)
                continue;

            std::vector<uint64_t> moves;
            switch (pt) {
                case bb::BLACK_PAWN:
                    moves = bb::generate_pawn_moves(pieceBB, emptySquares, enemyPieces, static_cast<bb::PieceType>(pt));
                    break;
                case bb::BLACK_KNIGHT:
                    moves = bb::generate_knight_moves(pieceBB, emptySquares, enemyPieces);
                    break;
                case bb::BLACK_KING: /// Unnecessary call right?
                    moves = bb::generate_king_moves(pieceBB, emptySquares, enemyPieces);
                    break;
                case bb::BLACK_BISHOP:
                    moves = bb::generate_bishop_moves(pieceBB, emptySquares, enemyPieces);
                    break;
                case bb::BLACK_ROOK:
                    moves = bb::generate_rook_moves(pieceBB, emptySquares, enemyPieces);
                    break;
                case bb::BLACK_QUEEN:
                    moves = bb::generate_queen_moves(pieceBB, emptySquares, enemyPieces);
                    break;
                default:
                    break;
            }
            for (uint64_t dest : moves) {
                attackMask |= dest;
            }
        }
        return (attackMask & whiteKing) != 0ULL;
    }

    // getValidMoves returns a fixed-size boolean mask (size 4672) indicating legal moves.
    // For each legal candidate move, we compute:
    //  - fromSquare, from the original piece bitboard difference.
    //  - toSquare similarly.
    //  - shift = toSquare - fromSquare is then used with getMovementType() to encode a move.
    // We then perform a temporary legality test using tempApplyActionToPieces.
    std::pair<std::array<bool, 4672>, bool> getValidMoves(const Chess::State &state) {
        std::array<bool, 4672> moveMask = {}; // all false by default
        moveMask.fill(false);

        // Obtain the global empty and enemy masks from the current state's pieces.
        // Getting white pieces moves so our enemy is black
        auto [emptySquares, enemyPieces] = getImportantSquares(state.pieces, Chess::BLACK);

        // Iterate over white piece types only (indices 0..5).
        for (int pt = 0; pt < 6; ++pt) {
            uint64_t pieceBB = state.pieces[pt];
            if (pieceBB == 0ULL)
                continue;

            // Call the move generator for this type.
            std::vector<uint64_t> candidateMoves;
            switch (pt) {
                case bb::WHITE_PAWN:
                    candidateMoves = bb::generate_pawn_moves(pieceBB, emptySquares, enemyPieces, static_cast<bb::PieceType>(pt));
                    // ————— EN PASSANT —————
                    if (state.flags.en_passant) {

                        uint64_t epRank5 = (uint64_t(state.flags.en_passant) << 32);

                        uint64_t pawns = pieceBB;
                        uint64_t canLeft = pawns & ((epRank5 << 1) & NO_A_FILE);
                        uint64_t canRight = pawns & ((epRank5 >> 1) & NO_H_FILE);

                        if (canLeft) {
                            uint64_t newBB = (pawns & ~canLeft) | (epRank5 << 8);
                            candidateMoves.push_back(newBB);
                        }
                        if (canRight) {
                            uint64_t newBB = (pawns & ~canRight) | (epRank5 << 8);
                            candidateMoves.push_back(newBB);
                        }
                    }
                    break;
                case bb::WHITE_KNIGHT:
                    candidateMoves = bb::generate_knight_moves(pieceBB, emptySquares, enemyPieces);
                    break;
                case bb::WHITE_KING:
                    candidateMoves = bb::generate_king_moves(pieceBB, emptySquares, enemyPieces);
                    // TODO: Check if currently in check, and then in middle travel square is in check
                    // ————— CASTLING —————
                    if (state.flags.castle_rights) {
                        uint8_t cr = state.flags.castle_rights;
                        // White
                        if (state.flags.turn == Chess::WHITE) {
                            // Queen side
                            if ((WHITE_Q_CASTLE & cr) && state.typeAtSquare[4] == bb::NO_PIECE && state.typeAtSquare[5] == bb::NO_PIECE && state.typeAtSquare[6] == bb::NO_PIECE)
                                candidateMoves.push_back(1ULL << 5);
                            // King side
                            if ((WHITE_K_CASTLE & cr) && state.typeAtSquare[2] == bb::NO_PIECE && state.typeAtSquare[1] == bb::NO_PIECE)
                                candidateMoves.push_back(1ULL << 1);
                        }
                        // Black
                        else {
                            // Queen side
                            if ((BLACK_Q_CASTLE & cr) && state.typeAtSquare[3] == bb::NO_PIECE && state.typeAtSquare[2] == bb::NO_PIECE && state.typeAtSquare[1] == bb::NO_PIECE)
                                candidateMoves.push_back(1ULL << 2);
                            // King side
                            if ((BLACK_K_CASTLE & cr) && state.typeAtSquare[6] == bb::NO_PIECE && state.typeAtSquare[5] == bb::NO_PIECE)
                                candidateMoves.push_back(1ULL << 6);
                        }
                    }
                    break;
                case bb::WHITE_BISHOP:
                    candidateMoves = bb::generate_bishop_moves(pieceBB, emptySquares, enemyPieces);
                    break;
                case bb::WHITE_ROOK:
                    candidateMoves = bb::generate_rook_moves(pieceBB, emptySquares, enemyPieces);
                    break;
                case bb::WHITE_QUEEN:
                    candidateMoves = bb::generate_queen_moves(pieceBB, emptySquares, enemyPieces);
                    break;
                default:
                    break;
            }

//            // Debugging
//            bool debugCheck = pt == 5 && !candidateMoves.empty();
//            if (debugCheck) state.print();

            // For each candidate move (each is a new bitboard state for that piece type after a move).
            for (uint64_t newBB : candidateMoves) {
                // Determine the "from" square as the bit that was lost.
                uint64_t from_bb = (pieceBB ^ newBB) & pieceBB;
                if (from_bb == 0ULL)
                    continue;
                // Determine the "to" square as the bit that was added.
                uint64_t to_bb = (pieceBB ^ newBB) & newBB;
                if (to_bb == 0ULL)
                    continue;
                int fromSquare = bb_utils::ctz(from_bb);
                int toSquare = bb_utils::ctz(to_bb);
                int shift = toSquare - fromSquare;
                // Use the MoveMapping function to resolve move type.
                int moveType = MoveMapping::getMovementType(shift, fromSquare, pt);
                if (moveType < 0)
                    continue;
                int action = moveType * 64 + fromSquare;

                // 49-0-7, 55 --- 48

                // Legality test: use tempApplyActionToPieces to obtain a temporary pieces array.
                auto tempPieces = StateTransition::tempApplyActionToPieces(state, action);
//                dbgPrintBoard(tempPieces);

                if (!isInCheck(tempPieces)) {
                    // Debugging capturing opponent king
                    bool captureKing = (to_bb & state.pieces[bb::BLACK_KING]) != 0;
                    if (captureKing) return {moveMask, true};

                    // Mark this action in our mask as legal.
                    if (action >= 0 && action < 4672) {
                        // If we're promoting, mask in those promotions (not the current action)
                        // PROMOTION LOGIC for white pawns reaching rank 8
                        if (pt == bb::WHITE_PAWN && (to_bb & RANK_8_MASK) != 0) {
                            auto promoTypes = MoveMapping::getPromotionMovementTypes(pt, to_bb, shift);
                            for (int promoMT: promoTypes) {
                                if (promoMT < 0) continue;
                                // legality already checked so mask in
                                action = promoMT * 64 + fromSquare;
                                moveMask[action] = true;
                            }
                        }
                        // Otherwise, moving a piece normally and only one action to mask in
                        else moveMask[action] = true;
                    } else {std::cout << "Action not in action space in getValidMoves" << std::endl;}
                }
            }
        }
        return {moveMask, false};
    }
} // namespace MoveGeneration
