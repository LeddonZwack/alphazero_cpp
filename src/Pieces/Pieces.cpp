#include "Pieces/Pieces.hpp"

namespace Chess {

    void Pieces::printPieceType(PieceType pt) {
        switch (pt) {
            case WHITE_PAWN:   std::cout << "WHITE PAWN";   break;
            case WHITE_KNIGHT: std::cout << "WHITE KNIGHT"; break;
            case WHITE_BISHOP: std::cout << "WHITE BISHOP"; break;
            case WHITE_ROOK:   std::cout << "WHITE ROOK";   break;
            case WHITE_QUEEN:  std::cout << "WHITE QUEEN";  break;
            case WHITE_KING:   std::cout << "WHITE KING";   break;
            case BLACK_PAWN:   std::cout << "BLACK PAWN";   break;
            case BLACK_KNIGHT: std::cout << "BLACK KNIGHT"; break;
            case BLACK_BISHOP: std::cout << "BLACK BISHOP"; break;
            case BLACK_ROOK:   std::cout << "BLACK ROOK";   break;
            case BLACK_QUEEN:  std::cout << "BLACK QUEEN";  break;
            case BLACK_KING:   std::cout << "BLACK KING";   break;
            default:           std::cout << "Unknown piece type"; break;
        }
        std::cout << "\n";
    }

} // namespace Chess
