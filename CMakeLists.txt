cmake_minimum_required(VERSION 3.26)
project(RLC__)

set(CMAKE_CXX_STANDARD 17)

add_executable(RLC__ main.cpp
#        include/Pieces/Pieces.hpp
#        src/Pieces/Pieces.cpp
#        include/Pieces/SimplePieces.hpp
#        include/Pieces/SlidingPieces.hpp
#        include/Pieces/Bishops.hpp
#        include/Pieces/Rooks.hpp
#        include/Pieces/Queens.hpp
#        include/Pieces/Kings.hpp
#        include/Pieces/Knights.hpp
#        include/Pieces/Pawns.hpp
        include/State.hpp
        include/MoveMapping.hpp
        src/State.cpp
        include/MoveGeneration.hpp
        src/MoveGeneration.cpp
        include/bitboard/bitboard_utils.hpp
        include/bitboard/piece_type.hpp
        include/bitboard/movegen_simple.hpp
        src/bitboard/movegen_simple.cpp
        include/bitboard/movegen_sliding.hpp
        src/bitboard/movegen_sliding.cpp
        tests/test_bitboard.cpp
        tests/test_bitboard.hpp
        include/StateTransition.hpp
        src/StateTransition.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include)
