cmake_minimum_required(VERSION 3.26)
project(RLC__)

set(CMAKE_CXX_STANDARD 17)

# Path to locally downloaded libtorch
set(CMAKE_PREFIX_PATH "/Users/leddonzwack/Desktop/Projects/LocalLibraries/libtorch")

# Find LibTorch
find_package(Torch REQUIRED)

add_executable(RLC__
        main.cpp
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
        src/StateTransition.cpp
        include/GameStatus.hpp
        src/GameStatus.cpp
        include/StateEncoder.hpp
        src/StateEncoder.cpp
        include/Network.hpp
        src/Network.cpp
        include/AlphaZeroTrainer.hpp
        src/AlphaZeroTrainer.cpp
        include/MCTS.hpp
        src/MCTS.cpp
        include/ModelInterface.hpp
        src/ModelInterface.cpp
        include/AlphaZeroController.hpp
        src/AlphaZeroController.cpp
        include/AZTypes.hpp)

# Include headers from project
include_directories(${CMAKE_SOURCE_DIR}/include)

# Link LibTorch
target_link_libraries(RLC__ "${TORCH_LIBRARIES}")
target_compile_options(RLC__ PRIVATE "${TORCH_CXX_FLAGS}")
