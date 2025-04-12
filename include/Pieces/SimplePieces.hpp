#ifndef SIMPLE_PIECES_HPP
#define SIMPLE_PIECES_HPP

#include "Pieces.hpp"

namespace Chess {

    class SimplePieces : public Pieces {
    public:
        // SimplePieces simply inherits the base Pieces functionality.
        explicit SimplePieces(PieceType t, uint64_t b = 0ULL)
                : Pieces(t, b) {}
    };

} // namespace Chess

#endif // SIMPLE_PIECES_HPP
