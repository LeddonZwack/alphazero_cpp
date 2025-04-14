#include "State.hpp"
#include "Pieces/Pawns.hpp"
#include "Pieces/Knights.hpp"
#include "Pieces/Bishops.hpp"
#include "Pieces/Rooks.hpp"
#include "Pieces/Queens.hpp"
#include "Pieces/Kings.hpp"

#if defined(__GNUG__) || defined(__clang__)
#include <cassert>
#endif

namespace Chess {

    // Initial position constants (using hexadecimal literals)
    constexpr BB INIT_WHITE_PAWNS   = 0x000000000000ff00ULL;
    constexpr BB INIT_WHITE_KNIGHTS = 0x0000000000000042ULL;
    constexpr BB INIT_WHITE_BISHOPS = 0x0000000000000024ULL;
    constexpr BB INIT_WHITE_ROOKS   = 0x0000000000000081ULL;
    constexpr BB INIT_WHITE_QUEEN   = 0x0000000000000010ULL;
    constexpr BB INIT_WHITE_KING    = 0x0000000000000008ULL;

    constexpr BB INIT_BLACK_PAWNS   = 0x00ff000000000000ULL;
    constexpr BB INIT_BLACK_KNIGHTS = 0x4200000000000000ULL;
    constexpr BB INIT_BLACK_BISHOPS = 0x2400000000000000ULL;
    constexpr BB INIT_BLACK_ROOKS   = 0x8100000000000000ULL;
    constexpr BB INIT_BLACK_QUEEN   = 0x1000000000000000ULL;
    constexpr BB INIT_BLACK_KING    = 0x0800000000000000ULL;

    State::State(int historyLen)
            : turn(WHITE),
              castle_flags(0x0F),    // All castling rights available.
              en_passant_flags(0ULL),
              repeated_moves_flags(0),
              no_progress_flags(0),
              no_progress_color_flags(-1),
              total_move_flags(0),
              history(),
              historyLength(historyLen),
              repeated_moves()
    {
        // Initialize pieces array with piece objects.
        pieces[WHITE_PAWN]   = std::make_unique<Pawns>(WHITE_PAWN, INIT_WHITE_PAWNS);
        pieces[WHITE_KNIGHT] = std::make_unique<Knights>(WHITE_KNIGHT, INIT_WHITE_KNIGHTS);
        pieces[WHITE_BISHOP] = std::make_unique<Bishops>(WHITE_BISHOP, INIT_WHITE_BISHOPS);
        pieces[WHITE_ROOK]   = std::make_unique<Rooks>(WHITE_ROOK, INIT_WHITE_ROOKS);
        pieces[WHITE_QUEEN]  = std::make_unique<Queens>(WHITE_QUEEN, INIT_WHITE_QUEEN);
        pieces[WHITE_KING]   = std::make_unique<Kings>(WHITE_KING, INIT_WHITE_KING);

        pieces[BLACK_PAWN]   = std::make_unique<Pawns>(BLACK_PAWN, INIT_BLACK_PAWNS);
        pieces[BLACK_KNIGHT] = std::make_unique<Knights>(BLACK_KNIGHT, INIT_BLACK_KNIGHTS);
        pieces[BLACK_BISHOP] = std::make_unique<Bishops>(BLACK_BISHOP, INIT_BLACK_BISHOPS);
        pieces[BLACK_ROOK]   = std::make_unique<Rooks>(BLACK_ROOK, INIT_BLACK_ROOKS);
        pieces[BLACK_QUEEN]  = std::make_unique<Queens>(BLACK_QUEEN, INIT_BLACK_QUEEN);
        pieces[BLACK_KING]   = std::make_unique<Kings>(BLACK_KING, INIT_BLACK_KING);

        // Initialize the board layout for typeAtSquare.
        // Here we follow the same indexing as Python:
        typeAtSquare = {
                // Rank 8 (index 0-7): White major pieces (here arranged with white-rook at index 0, knight at 1, bishop at 2, king at 3, queen at 4, bishop at 5, knight at 6, rook at 7)
                WHITE_ROOK,   WHITE_KNIGHT, WHITE_BISHOP, WHITE_KING,
                WHITE_QUEEN,  WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK,
                // Rank 7 (indexes 8-15): White pawns
                WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,
                WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,
                // Ranks 6-3 (indexes 16-47): empty
                -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1,
                // Rank 2 (indexes 48-55): Black pawns
                BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,
                BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,
                // Rank 1 (indexes 56-63): Black major pieces.
                BLACK_ROOK,   BLACK_KNIGHT, BLACK_BISHOP, BLACK_KING,
                BLACK_QUEEN,  BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK
        };

        initializeHistory();
    }

    // Copy constructor: value semantics (members deep copied).
    State::State(const State &other)
            : turn(other.turn),
              castle_flags(other.castle_flags),
              en_passant_flags(other.en_passant_flags),
              repeated_moves_flags(other.repeated_moves_flags),
              no_progress_flags(other.no_progress_flags),
              no_progress_color_flags(other.no_progress_color_flags),
              total_move_flags(other.total_move_flags),
              typeAtSquare(other.typeAtSquare),
              history(other.history),
              historyLength(other.historyLength),
              repeated_moves(other.repeated_moves)
    {
        // For pieces, perform a deep copy by re-creating new objects.
        // Here we rely on the piece type (stored in each object) to recreate the object.
        for (int i = 0; i < 12; ++i) {
            // Each piece class should offer a copy-like constructor.
            // For simplicity we call their constructors with the same type and board.
            PieceType pt = static_cast<PieceType>(i);
            BB boardVal = other.pieces[i]->board;  // Assume Pieces class has getBoard()
            switch (pt) {
                case WHITE_PAWN:
                    pieces[i] = std::make_unique<Pawns>(pt, boardVal);
                    break;
                case WHITE_KNIGHT:
                    pieces[i] = std::make_unique<Knights>(pt, boardVal);
                    break;
                case WHITE_BISHOP:
                    pieces[i] = std::make_unique<Bishops>(pt, boardVal);
                    break;
                case WHITE_ROOK:
                    pieces[i] = std::make_unique<Rooks>(pt, boardVal);
                    break;
                case WHITE_QUEEN:
                    pieces[i] = std::make_unique<Queens>(pt, boardVal);
                    break;
                case WHITE_KING:
                    pieces[i] = std::make_unique<Kings>(pt, boardVal);
                    break;
                case BLACK_PAWN:
                    pieces[i] = std::make_unique<Pawns>(pt, boardVal);
                    break;
                case BLACK_KNIGHT:
                    pieces[i] = std::make_unique<Knights>(pt, boardVal);
                    break;
                case BLACK_BISHOP:
                    pieces[i] = std::make_unique<Bishops>(pt, boardVal);
                    break;
                case BLACK_ROOK:
                    pieces[i] = std::make_unique<Rooks>(pt, boardVal);
                    break;
                case BLACK_QUEEN:
                    pieces[i] = std::make_unique<Queens>(pt, boardVal);
                    break;
                case BLACK_KING:
                    pieces[i] = std::make_unique<Kings>(pt, boardVal);
                    break;
                default:
                    break;
            }
        }
    }

    void State::initializeHistory() {
        history.clear();
        std::shared_ptr<State> self = returnStatePtr();
        for (int i = 0; i < historyLength; ++i) {
            history.push_back(self);  // weak_ptr stored
        }
    }

    std::shared_ptr<State> State::returnStatePtr() {
        return shared_from_this();
    }

} // namespace Chess
