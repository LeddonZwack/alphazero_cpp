#include "StateTransformer.hpp"
#include "Bitboard.hpp"
#include <cassert>
#include <sstream>
#include <iostream>
#include <string>

namespace Chess {

    // Helper: Concatenates a string representation of each piece’s bitboard.
    // This is used for the repeated-moves hash.
    static std::string concatenateBitboards(const std::array<std::unique_ptr<Pieces>, 12>& pieces) {
        std::stringstream ss;
        for (const auto& piece : pieces) {
            // Access the public member 'board' from each piece.
            ss << Bitboard::toString(piece->board) << "$";
        }
        return ss.str();
    }

    State& get_next_state(State &state, int action, const std::vector<MovementFunction> &movementFunctions) {

        // *** PREPROCESSING *** //

        // --- Determine the from-square and movement type ---
        int from_square   = action % 64;
        int movement_type = action / 64;
        MovementFunction movement_function = movementFunctions[movement_type];

        // --- Calculate FROM and TO bitboards ---
        BB from_bitboard = 1ULL << from_square;
        BB to_bitboard   = movement_function(from_bitboard);

        // --- Determine which piece is moving ---
        int from_piece_type = state.typeAtSquare[from_square];
        if (from_piece_type < 0 || from_piece_type >= static_cast<int>(state.pieces.size())) {
            std::cerr << "Invalid piece type at square " << from_square << ": " << from_piece_type << "\n";
            assert(false && "from_piece_type not found in state.pieces");
        }

        // *** UPDATING BITBOARDS *** //

        // --- Update piece bitboards: move the piece from its old to new position ---
        state.pieces[from_piece_type]->board = (state.pieces[from_piece_type]->board &
                                         Bitboard::complement(from_bitboard))| to_bitboard;

        // --- Edge case 1: CASTLING (for white king) ---
        if (from_piece_type == WHITE_KING && movement_type == 15) {
            // White king-side (short) castle:
            state.pieces[WHITE_ROOK]->board = (state.pieces[WHITE_ROOK]->board & 0xfffffffffffffffeULL)
                                       | 0x0000000000000004ULL;
        }
        else if (from_piece_type == WHITE_KING && movement_type == 43) {
            // White queen-side (long) castle:
            state.pieces[WHITE_ROOK]->board = (state.pieces[WHITE_ROOK]->board & 0xffffffffffffff7fULL)
                                       | 0x0000000000000010ULL;
        }

        // --- Edge case 2: CAPTURING ---
        bool capture_flag = false;
        // Use Bitboard::lsb_index to get the destination square index.
        int to_square = Bitboard::lsb_index(to_bitboard);
        int to_piece_type = state.typeAtSquare[to_square];
        if (to_piece_type != -1) {
            capture_flag = true;
            // Remove the captured piece from its bitboard.
            state.pieces[to_piece_type]->board &= Bitboard::complement(to_bitboard);
        }
        // Error
        else {
            std::cerr << "\n[ASSERT FAIL] Capture flag triggered, but destination square "
                      << to_square << " is empty (to_piece_type == -1).\n";
            std::cerr << "from_square: " << from_square << ", to_square: " << to_square << "\n";
            std::cerr << "Bitboard (from): ";
            Bitboard::print(from_bitboard);
            std::cerr << "Bitboard (to): ";
            Bitboard::print(to_bitboard);

            std::cerr << "Board at destination:\n";
            for (int i = 0; i < 64; ++i) {
                std::cerr << i << ": " << state.typeAtSquare[i] << ((i % 8 == 7) ? "\n" : ", ");
            }

            assert(false && "Capture attempted but no piece present at destination square");
        }

        // --- Edge case 3: EN PASSANT ---
        bool en_passant_flag = false;
        if (state.en_passant_flags > 0 && from_piece_type == WHITE_PAWN) {
            // Error
            if (capture_flag) {
                std::cerr << "\n[ASSERT FAIL] En passant attempted, but a capture was already registered.\n";
                std::cerr << "from_square: " << from_square << ", to_square: " << to_square << "\n";
                std::cerr << "from_piece_type: " << from_piece_type << "\n";
                std::cerr << "Bitboard (from): ";
                Bitboard::print(from_bitboard);
                std::cerr << "Bitboard (to): ";
                Bitboard::print(to_bitboard);

                std::cerr << "Current en_passant_flags: " << std::hex << state.en_passant_flags << std::dec << "\n";
                std::cerr << "typeAtSquare[" << to_square << "] = " << state.typeAtSquare[to_square] << "\n";

                assert(false && "En passant should not coincide with a normal capture");
            }

            // Shift the en passant flags left by 40 bits.
            BB en_passant_bb = state.en_passant_flags << 40;
            if ((en_passant_bb & to_bitboard) > 0) {
                // Remove the opponent pawn (assumed to be black) captured en passant.
                state.pieces[BLACK_PAWN]->board &= Bitboard::complement(en_passant_bb >> 8);
                en_passant_flag = true;
            }
        }

        // --- Edge case 4: PROMOTIONS ---
        int promotion_flag = -1;  // -1 signifies no promotion.
        if (movement_type == 64 || movement_type == 65 || movement_type == 66) {
            // Promote to knight.
            state.pieces[WHITE_KNIGHT]->board |= to_bitboard;
            state.pieces[WHITE_PAWN]->board   &= Bitboard::complement(to_bitboard);
            promotion_flag = WHITE_KNIGHT;
        }
        else if (movement_type == 67 || movement_type == 69) {
            // Promote to bishop.
            state.pieces[WHITE_BISHOP]->board |= to_bitboard;
            state.pieces[WHITE_PAWN]->board   &= Bitboard::complement(to_bitboard);
            promotion_flag = WHITE_BISHOP;
        }
        else if (movement_type == 70 || movement_type == 71 || movement_type == 72) {
            // Promote to rook.
            state.pieces[WHITE_ROOK]->board |= to_bitboard;
            state.pieces[WHITE_PAWN]->board &= Bitboard::complement(to_bitboard);
            promotion_flag = WHITE_ROOK;
        }
        else if (from_piece_type == WHITE_PAWN &&
                 (movement_type == 0 || movement_type == 7 || movement_type == 49) &&
                 ((to_bitboard & 0xff00000000000000ULL) > 0)) {
            // Promote to queen.
            state.pieces[WHITE_QUEEN]->board |= to_bitboard;
            state.pieces[WHITE_PAWN]->board  &= Bitboard::complement(to_bitboard);
            promotion_flag = WHITE_QUEEN;
        }

        // *** UPDATING TYPEATSQUARE *** //

        // --- Update the typeAtSquare board ---
        // Remove the piece from its source square.
        state.typeAtSquare[from_square] = -1;
        // Place the piece on its destination square.
        state.typeAtSquare[to_square] = from_piece_type;

        // Edge case 1: CASTLING
        // Adjust for castling: update rook positions on the board.
        if (from_piece_type == WHITE_KING && movement_type == 15) {
            state.typeAtSquare[0] = -1;
            state.typeAtSquare[2] = WHITE_ROOK;
        }
        else if (from_piece_type == WHITE_KING && movement_type == 43) {
            state.typeAtSquare[7] = -1;
            state.typeAtSquare[4] = WHITE_ROOK;
        }

        // Edge case 2: CAPTURING
        // Handled inherently by basic updating of typeAtSquare

        // Edge case 3: EN PASSANT
        // For en passant, remove the captured pawn from the board.
        if (en_passant_flag) {
            state.typeAtSquare[to_square - 8] = -1;
        }

        // Edge case 4: PROMOTIONS
        // If a promotion occurred, update the destination square’s piece type.
        if (promotion_flag != -1) {
            state.typeAtSquare[to_square] = promotion_flag;
        }

        // *** UPDATING FLAGS *** //

        // --- Repeated moves preprocessing ---
        // Reset repeated-moves if a capture or pawn move occurred.
        if (capture_flag || from_piece_type == WHITE_PAWN) {
            state.repeated_moves.clear();
        }
        // Concatenate the bitboard strings to generate a key.
        std::string concatBB = concatenateBitboards(state.pieces);
        // Check if seen key before and mutate
        if (state.repeated_moves.count(concatBB)) {
            state.repeated_moves[concatBB] += 1;
        } else {
            state.repeated_moves[concatBB] = 1;
        }

        // --- Begin updating flags ---

        // Update repeated_moves_flags based on the counter.
        if (state.repeated_moves[concatBB] == 2) {
            state.repeated_moves_flags = 0b01;
        }
        else if (state.repeated_moves[concatBB] == 3) {
            state.repeated_moves_flags = 0b10;
        }

        // Update en passant flag for a double pawn move.
        if (from_piece_type == WHITE_PAWN && movement_type == 1) {
            state.en_passant_flags = to_bitboard >> 24;
        } else {
            state.en_passant_flags = 0;
        }

        // Update castling rights.
        if (state.castle_flags > 0) {
            if (from_piece_type == WHITE_KING && state.turn == WHITE)
                state.castle_flags &= 0b1100;
            else if (from_piece_type == WHITE_KING && state.turn == BLACK)
                state.castle_flags &= 0b0011;
            else if (from_piece_type == WHITE_ROOK && (from_bitboard & 0x0000000000000001ULL) > 0 && state.turn == WHITE)
                state.castle_flags &= 0b1101;
            else if (from_piece_type == WHITE_ROOK && (from_bitboard & 0x0000000000000080ULL) > 0 && state.turn == WHITE)
                state.castle_flags &= 0b1110;
            else if (from_piece_type == WHITE_ROOK && (from_bitboard & 0x0000000000000001ULL) > 0 && state.turn == BLACK)
                state.castle_flags &= 0b0111;
            else if (from_piece_type == WHITE_ROOK && (from_bitboard & 0x0000000000000080ULL) > 0 && state.turn == BLACK)
                state.castle_flags &= 0b1011;
        }

        // Increment the move count after Black moves.
        if (state.turn == BLACK) {
            state.total_move_flags++;
        }

        // Update no-progress counter.
        if (from_piece_type == WHITE_PAWN || from_piece_type == BLACK_PAWN || capture_flag) {
            state.no_progress_color_flags = (state.turn == WHITE) ? WHITE : BLACK;
            state.no_progress_flags = 0;
        } else if (state.turn == state.no_progress_color_flags) {
            state.no_progress_flags++;
        }

        // Save history: remove the oldest entry and push a new one.
        state.history.pop_front();
        state.history.push_back(state.returnStatePtr());

        return state;
    }

} // namespace Chess
