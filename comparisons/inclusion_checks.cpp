#include <abstracton/mata_extensions.hpp>
#include <chrono>
#include <random>
#define INIT_CLOCKS() std::chrono::steady_clock::time_point begin, end;
#define TICK() begin = std::chrono::steady_clock::now();
#define TOCK(message) end = std::chrono::steady_clock::now(); std::cout << "Measured time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
#define TOCKM(message) end = std::chrono::steady_clock::now(); std::cout << "Time needed for " << message << ": " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;


int main(int argc, char** argv) {
    std::optional<unsigned int> seed = std::nullopt;
    int iterations = 5;
    bool user_seed = false;
    if (argc == 2) {
        seed = std::make_optional(atoi(argv[1]));
        iterations = 1;
        user_seed = true;
    }

    for (int i = 0; i < iterations; ++i) {
        size_t num_of_levels = 2;
        size_t num_of_states = 200;
        std::vector<size_t> alphabet_sizes = {2, 3};
        double states_trans_ratio_per_symbol = 10.0;
        double final_state_density = 0.1;
        if (!user_seed) {
            seed = std::make_optional(std::random_device{}());
        }
        mata::nft::Nft aut = mata::ext::builder::create_random_nft_tabakov_vardi(
            num_of_levels,
            num_of_states,
            alphabet_sizes,
            states_trans_ratio_per_symbol,
            final_state_density,
            seed
        );

        mata::EnumAlphabet alph0 = {0, 1};
        mata::EnumAlphabet alph1 = {0, 1, 2};
        std::vector<mata::Alphabet*> alphabets = {&alph0, &alph1};
        mata::nft::Run cex0, cex1, cex2;

        std::cout << "Seed: " << seed.value() << std::endl;

        INIT_CLOCKS();

        std::cout << "Default:" << std::endl;
        TICK();
        bool is_universal0 = mata::ext::is_universal_antichains(aut, alphabets, &cex0);
        TOCK();
        std::cout << "Result: " << is_universal0 << std::endl;

        std::cout << "Using inclusion:" << std::endl;
        TICK();
        bool is_universal1 = mata::ext::is_universal_antichains_by_inclusion(aut, alphabets, &cex1);
        TOCK();
        std::cout << "Result: " << is_universal1 << std::endl;

        std::cout << "Lazy (without subsumption):" << std::endl;
        TICK();
        bool is_universal2 = mata::ext::is_universal_lazy(aut, alphabets, &cex2);
        TOCK();
        std::cout << "Result: " <<is_universal2 << std::endl;

        if (is_universal0 != is_universal1 || is_universal0 != is_universal2) {
            std::cout << "ERROR: different answers for seed " << seed.value() << std::endl;
            return 0;
        }
    }
}
