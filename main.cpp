#include "tests/test_bitboard.hpp"
#include "AlphaZeroController.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
//    std::cout << "Running Bitboard Tests...\n";
//    run_all_bitboard_tests();

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <mode>\n"
                  << "  mode = train | play\n";
        return 1;
    }

    std::string mode = argv[1];

    // ── TODO: Populate these with real values or parse additional CLI args ──
    ControllerArgs args = {
            /* gameConfig */       {8, 8, 8, 4672},
            /* numResBlocks */     10,
            /* numHidden */        128,
            /* device */           torch::cuda::is_available() ? torch::kCUDA : torch::kCPU,
            /* learningRate */     1e-3,
            /* trainerArgs */      {
                                           3,   // num_iterations
                                           10,   // num_selfPlay_iterations
                                           500, // num_searches
                                           4,     // num_epochs
                                           64,    // batch_size
                                           1.0,   // temperature
                                           0.25,  // dirichlet_epsilon
                                           0.03,  // dirichlet_alpha
                                           1.41,   // C
                                           8      // historyLength
                                   }
    };
    // ───────────────────────────────────────────────────────────────────────

    AlphaZeroController controller(args);

    if (mode == "train") {
        controller.runTraining();
    } else if (mode == "play") {
        controller.runPlay();
    } else {
        std::cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }

    return 0;
}

//# include "tests/test_changePerspective.hpp"
//#include <iostream>
//#include <string>
//
//int main() {
//    tt::runTests();
//
//    return 0;
//}