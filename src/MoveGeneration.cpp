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

    // Compute the union of all pieces: empty squares = complement(white ∪ black),
    // enemyPieces = union of all black pieces (indices 6..11).
    std::pair<uint64_t, uint64_t> getImportantSquares(const std::array<uint64_t, 12> &pieces) {
        uint64_t whitePieces = 0ULL, blackPieces = 0ULL;
        for (int pt = 0; pt < 6; ++pt)
            whitePieces |= pieces[pt];
        for (int pt = 6; pt < 12; ++pt)
            blackPieces |= pieces[pt];
        uint64_t occupied = whitePieces | blackPieces;
        uint64_t emptySquares = ~occupied;
        return {emptySquares, blackPieces};
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

        auto [emptySquares, enemyPieces] = getImportantSquares(pieces);
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
                case bb::BLACK_KING:
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
        auto [emptySquares, enemyPieces] = getImportantSquares(state.pieces);

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
                    if (state.flags.en_passant > 0) {
                        uint64_t ep = uint64_t(state.flags.en_passant) << 32;  // en passant target square (rank 5)
                        uint64_t dest = ep << 8;  // where pawn lands (rank 6)

                        // LEFT capture: pawn must be to the right of ep square
                        uint64_t left_pawn = pieceBB & ((ep << 1) & ~FILE_H_MASK);
                        if (left_pawn)
                            candidateMoves.push_back((pieceBB & ~left_pawn) | dest);

                        // RIGHT capture: pawn must be to the left of ep square
                        uint64_t right_pawn = pieceBB & ((ep >> 1) & ~FILE_A_MASK);
                        if (right_pawn)
                            candidateMoves.push_back((pieceBB & ~right_pawn) | dest);
                    }
                    break;
                case bb::WHITE_KNIGHT:
                    candidateMoves = bb::generate_knight_moves(pieceBB, emptySquares, enemyPieces);
                    break;
                case bb::WHITE_KING:
                    candidateMoves = bb::generate_king_moves(pieceBB, emptySquares, enemyPieces);
                    // ————— CASTLING —————
                    {
                        unsigned rights = (state.flags.turn ? (state.flags.castle_rights >> 2) : state.flags.castle_rights) & 0b11;
                        // white’s castle bits are the low two bits of castle_rights
                        if (rights == 0) break;

                        // white king‑side (O‑O)
                        if (state.flags.turn == 0 && state.typeAtSquare[3] == bb::NO_PIECE && state.typeAtSquare[2] == bb::NO_PIECE)
                            candidateMoves.push_back(1ULL << 1);
                        // white queen‑side (O‑O‑O)
                        if (state.flags.turn == 0 && state.typeAtSquare[4] == bb::NO_PIECE && state.typeAtSquare[3] == bb::NO_PIECE)
                            candidateMoves.push_back(1ULL << 5);

                        // black king‑side (O‑O)
                        if (state.flags.turn == 1 && state.typeAtSquare[5] == bb::NO_PIECE && state.typeAtSquare[4] == bb::NO_PIECE)
                            candidateMoves.push_back(1ULL << 6);
                        // white queen‑side (O‑O‑O)
                        if (state.flags.turn == 1 && state.typeAtSquare[4] == bb::NO_PIECE && state.typeAtSquare[3] == bb::NO_PIECE)
                            candidateMoves.push_back(1ULL << 2);

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

                // Legality test: use tempApplyActionToPieces to obtain a temporary pieces array.
                auto tempPieces = StateTransition::tempApplyActionToPieces(state, action);

                // Debug check
                if (tempPieces[bb::WHITE_KING] == 0) {
                    std::cout << "WHITE KING NOT FOUND. moveType: " << moveType << "fromSquare: " << fromSquare << "\n \n";
                    std::string pieceNames[12] = {
                            "WHITE_PAWN", "WHITE_KNIGHT", "WHITE_BISHOP", "WHITE_ROOK",
                            "WHITE_QUEEN", "WHITE_KING",  "BLACK_PAWN",   "BLACK_KNIGHT",
                            "BLACK_BISHOP", "BLACK_ROOK", "BLACK_QUEEN",  "BLACK_KING"
                    };

                    std::cout << "──── Piece Bitboards ────\n";
                    for (int pt = 0; pt < 12; ++pt) {
                        std::cout << pieceNames[pt] << ":\n";
                        std::cout << "  Bitboard (bin): " << std::bitset<64>(tempPieces[pt]) << "\n";
                        std::cout << "  Bitboard (hex): 0x" << std::hex << tempPieces[pt] << std::dec << "\n\n";
                    }
                    std::cout << "─────────────────────────\n";
                    return {moveMask, true};
                }

                if (!isInCheck(tempPieces)) {
                    // Mark this action in our mask as legal.
                    if (action >= 0 && action < 4672) {
                        moveMask[action] = true;

                        // PROMOTION LOGIC for white pawns reaching rank 8
                        if (pt == bb::WHITE_PAWN) {
                            auto promoTypes = MoveMapping::getPromotionMovementTypes(pt, to_bb, shift);
                            for (int promoMT: promoTypes) {
                                if (promoMT < 0) continue;
                                // legality already checked so mask in
                                action = promoMT * 64 + fromSquare;
                                moveMask[action] = true;
                            }
                        }
                    } else {std::cout << "Action not in action space in getValidMoves" << std::endl;}
                }
            }
        }
        return {moveMask, false};
    }
} // namespace MoveGeneration
