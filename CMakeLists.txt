cmake_minimum_required(VERSION 3.26)
project(RLC__)

set(CMAKE_CXX_STANDARD 17)

add_executable(RLC__ main.cpp
        src/Bitboard.cpp
        include/Bitboard.hpp
        include/Pieces/Pieces.hpp
        src/Pieces/Pieces.cpp
        include/Pieces/SimplePieces.hpp
        include/Pieces/SlidingPieces.hpp
        include/Pieces/Bishops.hpp
        include/Pieces/Rooks.hpp
        include/Pieces/Queens.hpp
        include/Pieces/Kings.hpp
        include/Pieces/Knights.hpp
        include/Pieces/Pawns.hpp
        tests/test_pieces.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include)
