#include "StateTransition.hpp"
#include <cassert>
#include <algorithm>

namespace StateTransition {

    // Main function: given a current state and an action, return a new State updated accordingly.
    void getNextState(Chess::State &currState, int action) {

        /// *** PREPROCESSING *** ///

        // Decode the action.
        int fromSquare = action % 64;
        int moveType   = action / 64;

        // Compute bitboard with one bit at fromSquare.
        uint64_t from_bb = 1ULL << fromSquare;
        // Apply movement: use our Module A function.
        uint64_t to_bb = MoveMapping::applyMovement(from_bb, moveType);

        // Determine the moving piece type using the typeAtSquare array.
        int movingPieceType = static_cast<int>(currState.typeAtSquare[fromSquare]);
        assert(movingPieceType >= 0 && movingPieceType < 12);

        /// *** UPDATING BITBOARDS *** ///

        // Update: remove the piece from the from-square & add at new to-square
        currState.pieces[movingPieceType] &= ~from_bb;
        currState.pieces[movingPieceType] |= to_bb;

        // Handle edge cases:
        // 1. Castling (we assume specific moveType values designate castling).
        if (movingPieceType == bb::WHITE_KING) {
            if (moveType == 15) {
                // King-side castle: update rook.
                currState.pieces[bb::WHITE_ROOK] &= ~(1ULL << 0);
                currState.pieces[bb::WHITE_ROOK] |= (1ULL << 2);
            } else if (moveType == 43) {
                // Queen-side castle.
                currState.pieces[bb::WHITE_ROOK] &= ~(1ULL << 7);
                currState.pieces[bb::WHITE_ROOK] |= (1ULL << 4);
            }
        }

        // 2. Capture: if a piece is at toSquare, then a capture occurred
        bool captureOccurred(false);
        int toSquare = 63 - bb_utils::ctz(to_bb);
        if (currState.typeAtSquare[toSquare] != bb::NO_PIECE)
        {
            captureOccurred = true;
            currState.pieces[static_cast<int>(currState.typeAtSquare[toSquare])] &= ~to_bb;
        }

        // 3. En passant: if opponent pawn is captured through en passant,
        //    we need to remove an opponent pawn from square below toSquare.
        int capturedByEnPassant = -1;
        if (currState.flags.en_passant > 0 && movingPieceType == bb::WHITE_PAWN) { // TODO: potential improvement from > 0 to != 0ULL or equivalent
            // En passant was possible and moving a white pawn, now check if actually occurred for this action
            uint64_t en_passant_bb = static_cast<uint64_t>(currState.flags.en_passant) << 40;
            if ((en_passant_bb & to_bb) > 0) {
                en_passant_bb = en_passant_bb >> 8;
                capturedByEnPassant = 63 - (__builtin_clzll(en_passant_bb));
                currState.pieces[bb::BLACK_PAWN] &= ~(en_passant_bb);
            }
        }

        // 4. Promotions: if a pawn reaches the last rank.
        int pawnPromoted = -1;
        if (moveType == 64 || moveType == 65 || moveType == 66) {
            currState.pieces[bb::WHITE_PAWN] &= ~to_bb;
            currState.pieces[bb::WHITE_KNIGHT] |= to_bb;
            pawnPromoted = bb::WHITE_KNIGHT;
        } else if (moveType == 67 || moveType == 68 || moveType == 69) {
            currState.pieces[bb::WHITE_PAWN] &= ~to_bb;
            currState.pieces[bb::WHITE_BISHOP] |= to_bb;
            pawnPromoted = bb::WHITE_BISHOP;
        } else if (moveType == 70 || moveType == 71 || moveType == 72) {
            currState.pieces[bb::WHITE_PAWN] &= ~to_bb;
            currState.pieces[bb::WHITE_QUEEN] |= to_bb;
            pawnPromoted = bb::WHITE_QUEEN;
        }

        /// *** UPDATING TYPEATSQUARE *** ///

        // Update the moving piece.
        currState.typeAtSquare[fromSquare] = static_cast<uint8_t>(bb::NO_PIECE);
        currState.typeAtSquare[toSquare]   = static_cast<uint8_t>(movingPieceType);

        // Handle edge cases:
        // 1. Castling
        if (movingPieceType == bb::WHITE_KING) {
            if (moveType == 15) {
                // King-side castle: update rook.
                currState.typeAtSquare[0] = static_cast<uint8_t>(bb::NO_PIECE);
                currState.typeAtSquare[2] = static_cast<uint8_t>(bb::WHITE_ROOK);
            } else if (moveType == 43) {
                // Queen-side castle.
                currState.typeAtSquare[7] = static_cast<uint8_t>(bb::NO_PIECE);
                currState.typeAtSquare[4] = static_cast<uint8_t>(bb::WHITE_ROOK);
            }
        }

        // 2. Capture: Handled inherently by updating the moving piece.

        // 3. En Passant
        if (capturedByEnPassant != -1) {
            currState.typeAtSquare[capturedByEnPassant] = static_cast<uint8_t>(bb::NO_PIECE);
        }

        // 4. Promotions
        if (pawnPromoted != -1) {
            currState.typeAtSquare[toSquare] = static_cast<uint8_t>(pawnPromoted); // TODO: Need to static cast for all newTAS
        }

        /// *** UPDATING FLAGS *** ///

        // Not doing anything with repeated states

        // Turn
        currState.flags.turn = ~currState.flags.turn;

        // Castle Rights
        if (currState.flags.castle_rights > 0) {
            // King moved
            if (movingPieceType == bb::WHITE_KING){
                // White's turn
                if (currState.flags.turn == Chess::WHITE) {
                    currState.flags.castle_rights &= 0b1100;
                }
                // Black's turn
                else {
                    currState.flags.castle_rights &= 0b0011;
                }
            }
            // Rook moved
            else if (movingPieceType == bb::WHITE_ROOK) {
                // White's turn
                if (currState.flags.turn == Chess::WHITE) {
                    // King side rook moved
                    if ((from_bb & (1ULL << 0)) > 0) {
                        currState.flags.castle_rights &= 0b1101;
                    }
                    // Queen side rook moved
                    else if ((from_bb & (1ULL << 7)) > 0) {
                        currState.flags.castle_rights &= 0b1110;
                    }
                }
                    // Black's turn
                else {
                    // King side rook moved
                    if ((from_bb & (1ULL << 0)) > 0) {
                        currState.flags.castle_rights &= 0b0111;
                    }
                        // Queen side rook moved
                    else if ((from_bb & (1ULL << 7)) > 0) {
                        currState.flags.castle_rights &= 0b1011;
                    }
                }
            }
        }

        // En Passant
        if (movingPieceType == bb::WHITE_PAWN && moveType == 1) {
            currState.flags.en_passant = static_cast<uint8_t>((to_bb >> 24) & 0xFF);
        }

        // Half Move count & No Progress Side
        if (movingPieceType == bb::WHITE_PAWN || captureOccurred) {
            currState.flags.no_progress_side = currState.flags.turn;
            currState.flags.half_move_count = 0;
        } else if (currState.flags.turn == currState.flags.no_progress_side) {
            currState.flags.half_move_count++;
        }

        // Total Move Count
        if (currState.flags.turn == Chess::BLACK) {
            currState.flags.total_move_count++;
        }

        /// *** CHANGE PERSPECTIVE *** ///

        // Do a 180º flip of the board
        changePerspective(currState.pieces, currState.typeAtSquare, currState.flags.en_passant);

        /// *** UPDATE ZOBRIST HASH *** ///
        currState.zobrist_hash = currState.computeZobrist();
    }

    Chess::State getCopyNextState(const Chess::State &currState, int action) {
        // Create a copy of current State
        Chess::State newState(currState.pieces, currState.typeAtSquare, currState.flags, currState.zobrist_hash);

        // Apply action to new state and return
        getNextState(newState, action);
        return newState;
    }


    // TODO: Find a better solution to this down the line, but fine for now.
    // Redone logic from getNextState. Will circle back to it later
    std::array<uint64_t, 12> tempApplyActionToPieces(const Chess::State &currState, int action)
    {
        /// *** COPIES FOR NEW STATE *** ///

        // Work with a copy of the current state's bitboards.
        std::array<uint64_t, 12> newPieces = currState.pieces;

        /// *** PREPROCESSING *** ///

        // Decode the action.
        int fromSquare = action % 64;
        int moveType   = action / 64;

        // Compute bitboard with one bit at fromSquare.
        uint64_t from_bb = 1ULL << fromSquare;
        // Apply movement: use our Module A function.
        uint64_t to_bb = MoveMapping::applyMovement(from_bb, moveType);

        // Determine the moving piece type using the typeAtSquare array.
        int movingPieceType = static_cast<int>(currState.typeAtSquare[fromSquare]);
        assert(movingPieceType >= 0 && movingPieceType < 12);

        /// *** UPDATING BITBOARDS *** ///

        // Update: remove the piece from the from-square & add at new to-square
        newPieces[movingPieceType] &= ~from_bb;
        newPieces[movingPieceType] |= to_bb;

        // Handle edge cases:
        // 1. Castling (we assume specific moveType values designate castling).
        if (movingPieceType == bb::WHITE_KING) {
            if (moveType == 15) {
                // King-side castle: update rook.
                newPieces[bb::WHITE_ROOK] &= ~(1ULL << 0);
                newPieces[bb::WHITE_ROOK] |= (1ULL << 2);
            } else if (moveType == 43) {
                // Queen-side castle.
                newPieces[bb::WHITE_ROOK] &= ~(1ULL << 7);
                newPieces[bb::WHITE_ROOK] |= (1ULL << 4);
            }
        }

        // 2. Capture: if a piece is at toSquare, then a capture occurred
//        bool captureOccurred(false);
        int toSquare = 63 - (__builtin_clzll(to_bb)); // or use popcount on (to_bb-1)
        if (currState.typeAtSquare[toSquare] != bb::NO_PIECE)
        {
//            captureOccurred = true;
            newPieces[static_cast<int>(currState.typeAtSquare[toSquare])] &= ~to_bb;
        }

        // 3. En passant: if opponent pawn is captured through en passant,
        //    we need to remove an opponent pawn from square below toSquare.
//        int capturedByEnPassant = -1;
        if (currState.flags.en_passant > 0 && movingPieceType == bb::WHITE_PAWN) {
            // En passant was possible and moving a white pawn, now check if actually occurred for this action
            uint64_t en_passant_bb = static_cast<uint64_t>(currState.flags.en_passant) << 40;
            if ((en_passant_bb & to_bb) > 0) {
                en_passant_bb = en_passant_bb >> 8;
//                capturedByEnPassant = 63 - (__builtin_clzll(en_passant_bb));
                newPieces[bb::BLACK_PAWN] &= ~(en_passant_bb);
            }
        }

        // 4. Promotions: if a pawn reaches the last rank.
//        int pawnPromoted = -1;
        if (moveType == 64 || moveType == 65 || moveType == 66) {
            newPieces[bb::WHITE_PAWN] &= ~to_bb;
            newPieces[bb::WHITE_KNIGHT] |= to_bb;
//            pawnPromoted = bb::WHITE_KNIGHT;
        } else if (moveType == 67 || moveType == 68 || moveType == 69) {
            newPieces[bb::WHITE_PAWN] &= ~to_bb;
            newPieces[bb::WHITE_KNIGHT] |= to_bb;
//            pawnPromoted = bb::WHITE_BISHOP;
        } else if (moveType == 70 || moveType == 71 || moveType == 72) {
            newPieces[bb::WHITE_PAWN] &= ~to_bb;
            newPieces[bb::WHITE_KNIGHT] |= to_bb;
//            pawnPromoted = bb::WHITE_QUEEN;
        }

        return newPieces;
    }

    // Update the repeated states flag for a given state
    void updateRepeatedStateFlag(Chess::State &currState, const std::unordered_map<uint64_t, uint8_t>& repetitionMap) {
        int count = static_cast<int>(repetitionMap.at(currState.zobrist_hash));
        if (count == 2) currState.flags.repeated_state = 0b01;
        else if (count == 3) currState.flags.repeated_state = 0b10;
    }


    // --- Perspective / State Transformation Functions ---

    // changePerspective performs a full 180° rotation of the board.
    void changePerspective(std::array<uint64_t, 12> &pieces, std::array<uint8_t, 64> &typeAtSquare, uint8_t &en_passant) {
        // Rotate each bitboard 180° (equivalent to reversing the bits).
        for (int pt = 0; pt < 12; ++pt) {
            pieces[pt] = flip180(pieces[pt]);
        }

        // Swap white and black bitboard arrays.
        std::swap(pieces[0], pieces[6]);   // Pawns.
        std::swap(pieces[1], pieces[7]);   // Knights.
        std::swap(pieces[2], pieces[8]);   // Bishops.
        std::swap(pieces[3], pieces[9]);   // Rooks.
        std::swap(pieces[4], pieces[10]);  // Queens.
        std::swap(pieces[5], pieces[11]);  // Kings.

        // Rebuild typeAtSquare by performing a full 180° rotation:
        std::array<uint8_t, 64> newType;
        for (int i = 0; i < 64; ++i) {
            // The piece that was at index (63 - i) is moved and its color is flipped.
            uint8_t oldPiece = typeAtSquare[63 - i];
            if (oldPiece == bb::NO_PIECE) {
                newType[i] = bb::NO_PIECE;
            } else {
                newType[i] = static_cast<uint8_t>(flipPieceType(oldPiece));
            }
        }
        typeAtSquare = newType;

        // Flip en passant flag over center
        if (en_passant != 0) {
            en_passant = 1u << (7 - bb_utils::ctz(en_passant));
        }
    }

} // namespace StateTransition
