#include <mata/nft/nft.hh>
#include <abstracton/mata_extensions.hpp>

int main() {
    using namespace mata;
    using namespace mata::nft;
    for (int i = 0; i < 100; ++i) {
        std::cout << "iteration " << i << "...\n";
        size_t num_of_levels = 3;
        size_t num_of_states = 10;
        std::vector<size_t> alphabet_sizes = {3, 4, 2};
        double states_trans_ratio_per_symbol = 0.5;
        double final_state_density = 0.5;
        std::cout << "\tcreating nft..." << std::endl;
        Nft nft = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density);
        // std::cout << nft.print_to_dot() << std::endl;

        std::cout << "\tdeterminizing using ext..." << std::endl;
        Nft nft_det_ext = mata::ext::determinize(nft);
        // std::cout << nft_det_ext.print_to_dot() << std::endl;
        std::cout << "\tdeterminizing using mata..." << std::endl;
        Nft nft_det_mata = mata::nft::determinize(nft);
        // std::cout << nft_det_mata.print_to_dot() << std::endl;
        std::cout << "\tchecking determinism of ext..." << std::endl;
        assert(nft_det_ext.is_deterministic());
        std::cout << "\tchecking determinism of mata..." << std::endl;
        assert(nft_det_mata.is_deterministic());
        std::cout << "\tchecking correctness of ext..." << std::endl;
        assert(mata::nft::are_equivalent(nft, nft_det_ext));
        std::cout << "\tchecking correctness of mata..." << std::endl;
        assert(mata::nft::are_equivalent(nft, nft_det_mata));
        std::cout << "\tchecking equivalence..." << std::endl;
        assert(mata::nft::are_equivalent(nft_det_mata, nft_det_ext));
    }
}
