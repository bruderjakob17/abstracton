#include <abstracton/mata_extensions.hpp>
#include <chrono>
#include <random>

int main() {
    for (int i = 0; i < 5; ++i) {
        size_t num_of_levels = 2;
        size_t num_of_states = 2000;
        std::vector<size_t> alphabet_sizes = {2, 3};
        double states_trans_ratio_per_symbol = 10.0;
        double final_state_density = 0.1;
        std::optional<unsigned int> seed = std::make_optional(std::random_device{}());
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
        mata::nft::Run cex0;
        mata::nft::Run cex1;

        std::cout << "Seed: " << seed.value() << std::endl;

        std::cout << "Default:" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        bool is_universal = mata::ext::is_universal_antichains(aut, alphabets, &cex0);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "Result: " << is_universal << std::endl;
        std::cout << "Calculated in " << duration.count() << " microseconds" << std::endl;

        std::cout << "Using inclusion:" << std::endl;
        auto start2 = std::chrono::high_resolution_clock::now();
        bool is_universal2 = mata::ext::is_universal_antichains_by_inclusion(aut, alphabets, &cex1);
        auto end2 = std::chrono::high_resolution_clock::now();
        auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
        std::cout << "Result: " << is_universal2 << std::endl;
        std::cout << "Calculated in " << duration2.count() << " microseconds" << std::endl;

        if (is_universal != is_universal2) {
            std::cout << "ERROR: different answers for seed " << seed.value() << std::endl;
            return 0;
        }
    }
}
