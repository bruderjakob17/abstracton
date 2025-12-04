#include <catch2/catch_test_macros.hpp>
#include <mata/nft/nft.hh>
#include <mata/parser/re2parser.hh>
#include <abstracton/mata_extensions.hpp>
#include <algorithm>

TEST_CASE( "Identity on Alphabet correct", "[create_identity]" ) {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    Nft id {create_identity(EnumAlphabet {'0', '1'})};

    REQUIRE(id.get_words(4) == std::set<Word> {
        {},
        {'0', '0'},
        {'1', '1'},
        {'0', '0', '0', '0'},
        {'0', '0', '1', '1'},
        {'1', '1', '0', '0'},
        {'1', '1', '1', '1'}
    });
}

TEST_CASE("Identity on Language correct", "[create_identity]") {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    Nfa aut = mata::parser::create_nfa("(a|b)*cb*");

    Nft id {create_identity(aut)};

    REQUIRE(id.get_words(4) == std::set<Word> {
        {'c', 'c'},
        {'c', 'c', 'b', 'b'},
        {'a', 'a', 'c', 'c'},
        {'b', 'b', 'c', 'c'},
    });
}

TEST_CASE("Nft determinization", "[mata::ext::determinize]") {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    // length-preserving abstraction framework:
    // - a## encodes {000, 111} (i.e. all letters are the same)
    // - b## encodes {000} (i.e. all letters are 0)
    // so, transducer needs to accept (a##, 000), (a##, 111), (b##, 000)
    Nft af {Nft::with_levels(2)};
    State af_init {af.add_state() };
    af.initial.insert(af_init);
    State af_1 {af.add_transition(af_init, {'a', '0'})};
    af.add_transition(af_init, {'b', '0'}, af_1);
    State af_2 {af.add_transition(af_init, {'a', '1'})};
    State af_1x {af.add_transition(af_1, {'#', '0'})};
    State af_e {af.add_transition(af_1x, {'#', '0'})};
    State af_2x {af.add_transition(af_2, {'1', '#'})};
    af.add_transition(af_2x, {'1', '#'}, af_e);
    af.final.insert(af_e);

    std::cout << af.print_to_dot(true) << std::endl;
    Nft daf {mata::ext::determinize(af)};
    std::cout << daf.print_to_dot(true) << std::endl;

    REQUIRE(mata::nft::are_equivalent(af, daf));
    REQUIRE(daf.is_deterministic());
}

TEST_CASE("Nft minimization", "[mata::ext::minimize]") {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    Nft aut = Nft::with_levels(2, 4, { 0 }, { 3 });
    aut.add_transition(0, {'0', '0'}, 1);
    aut.add_transition(0, {'1', '1'}, 2);
    aut.add_transition(2, {'1', '1'}, 1);
    aut.add_transition(1, {'1', '1'}, 2);
    aut.add_transition(1, {'0', '1'}, 1);
    aut.add_transition(2, {'0', '1'}, 2);
    aut.add_transition(1, {'0', '0'}, 3);
    aut.add_transition(2, {'0', '0'}, 3);

    std::cout << aut.print_to_dot(true) << std::endl;
    Nft maut {mata::ext::minimize(aut)};
    std::cout << maut.print_to_dot(true) << std::endl;

    REQUIRE(mata::nft::are_equivalent(aut, maut));
    REQUIRE(maut.num_of_states_with_level(0) == 3);
}

TEST_CASE("Create Tabakov-Vardi NFT") {
    size_t num_of_levels;
    size_t num_of_states;
    std::vector<size_t> alphabet_sizes;
    double states_trans_ratio_per_symbol;
    double final_state_density;
    std::optional<unsigned int> seed{ std::nullopt };

    // NOTE all checks for number of transitions are commented out, as the number of counted transitions in the NFA representing the NFT is not determined uniquely by the number of transitions in the NFT

    SECTION("EMPTY") {
        num_of_levels = 0;
        num_of_states = 0;
        alphabet_sizes = {0, 0};
        states_trans_ratio_per_symbol = 0;
        final_state_density = 0;

        mata::nft::Nft nft = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density);
        CHECK(nft.levels.num_of_levels == 0);
        CHECK(nft.num_of_states_with_level(0) == 0);
        CHECK(nft.initial.size() == 0);
        CHECK(nft.final.size() == 0);
        CHECK(nft.delta.empty());
    }

    SECTION("3-10-5-0.5-0.5") {
        num_of_levels = 3;
        num_of_states = 10;
        alphabet_sizes = {3, 4, 5};
        states_trans_ratio_per_symbol = 0.5;
        final_state_density = 0.5;

        mata::nft::Nft nft = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density);
        CHECK(nft.levels.num_of_levels == 3);
        CHECK(nft.num_of_states_with_level(0) == num_of_states);
        CHECK(nft.initial.size() == 1);
        CHECK(nft.final.size() == 5);
        CHECK(nft.delta.get_used_symbols().size() == (*std::max_element(alphabet_sizes.begin(), alphabet_sizes.end())));
        // CHECK(nft.delta.num_of_transitions() == 300);
    }

    SECTION("Min final") {
        num_of_levels = 2;
        num_of_states = 10;
        alphabet_sizes = {3, 4};
        states_trans_ratio_per_symbol = 0.5;
        final_state_density = 0.0001;

        mata::nft::Nft nft = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density, seed);
        CHECK(nft.levels.num_of_levels == 2);
        CHECK(nft.num_of_states_with_level(0) == num_of_states);
        CHECK(nft.initial.size() == 1);
        CHECK(nft.final.size() == 1);
        CHECK(nft.delta.get_used_symbols().size() == (*std::max_element(alphabet_sizes.begin(), alphabet_sizes.end())));
        // CHECK(nft.delta.num_of_transitions() == 60);
    }

    SECTION("Max final") {
        num_of_levels = 2;
        num_of_states = 10;
        alphabet_sizes = {3, 4};
        states_trans_ratio_per_symbol = 0.5;
        final_state_density = 1.0;

        mata::nft::Nft nft = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density);
        CHECK(nft.levels.num_of_levels == 2);
        CHECK(nft.num_of_states_with_level(0) == num_of_states);
        CHECK(nft.initial.size() == 1);
        CHECK(nft.final.size() == num_of_states);
        CHECK(nft.delta.get_used_symbols().size() == (*std::max_element(alphabet_sizes.begin(), alphabet_sizes.end())));
        // CHECK(nft.delta.num_of_transitions() == 60);
    }

    SECTION("Min transitions") {
        num_of_levels = 2;
        num_of_states = 10;
        alphabet_sizes = {3, 4};
        states_trans_ratio_per_symbol = 0;
        final_state_density = 0.5;

        mata::nft::Nft nft = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density);
        CHECK(nft.levels.num_of_levels == 2);
        CHECK(nft.num_of_states_with_level(0) == num_of_states);
        CHECK(nft.initial.size() == 1);
        CHECK(nft.final.size() == 5);
        CHECK(nft.delta.get_used_symbols().size() == 0);
        // CHECK(nft.delta.num_of_transitions() == 0);
    }

    SECTION("Max transitions") {
        num_of_levels = 2;
        num_of_states = 10;
        alphabet_sizes = {3, 4};
        states_trans_ratio_per_symbol = 10;
        final_state_density = 0.5;

        mata::nft::Nft nft = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density);
        CHECK(nft.levels.num_of_levels == 2);
        CHECK(nft.num_of_states_with_level(0) == num_of_states);
        CHECK(nft.initial.size() == 1);
        CHECK(nft.final.size() == 5);
        CHECK(nft.delta.get_used_symbols().size() == (*std::max_element(alphabet_sizes.begin(), alphabet_sizes.end())));
        // CHECK(nft.delta.num_of_transitions() == 1200);
    }

    SECTION("BIG") {
        num_of_levels = 2;
        num_of_states = 200;
        alphabet_sizes = {10, 10};
        states_trans_ratio_per_symbol = 5;
        final_state_density = 1;

        mata::nft::Nft nft = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density);
        CHECK(nft.num_of_states_with_level(0) == num_of_states);
        CHECK(nft.initial.size() == 1);
        CHECK(nft.final.size() == num_of_states);
        CHECK(nft.delta.get_used_symbols().size() == (*std::max_element(alphabet_sizes.begin(), alphabet_sizes.end())));
        // CHECK(nft.delta.num_of_transitions() == 100000);
    }

    SECTION("Same seed results in same NFA.") {
        num_of_levels = 2;
        num_of_states = 10;
        alphabet_sizes = {3, 4};
        states_trans_ratio_per_symbol = 0.5;
        final_state_density = 0.5;
        std::optional<unsigned int> seed1{ 3171643142 };
        std::optional<unsigned int> seed2{ 4283451011 };

        mata::nft::Nft nft1_1 = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density, seed1);
        mata::nft::Nft nft1_2 = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density, seed1);
        mata::nft::Nft nft2 = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density, seed2);
        CHECK(nft1_1.is_identical(nft1_2));
        CHECK(!nft1_1.is_identical(nft2));
    }

    SECTION("Throw runtime_error. transition_density < 0") {
        num_of_levels = 2;
        num_of_states = 10;
        alphabet_sizes = {3, 4};
        states_trans_ratio_per_symbol = static_cast<double>(-0.1);
        final_state_density = 0.5;

        CHECK_THROWS_AS(mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density), std::runtime_error);
    }

    SECTION("Throw runtime_error. transition_density > num_of_states") {
        num_of_levels = 2;
        num_of_states = 10;
        alphabet_sizes = {3, 4};
        states_trans_ratio_per_symbol = 11;
        final_state_density = 0.5;

        CHECK_THROWS_AS(mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density), std::runtime_error);
    }

    SECTION("Throw runtime_error. final_state_density < 0") {
        num_of_levels = 2;
        num_of_states = 10;
        alphabet_sizes = {3, 4};
        states_trans_ratio_per_symbol = 0.5;
        final_state_density = static_cast<double>(-0.1);

        CHECK_THROWS_AS(mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density), std::runtime_error);
    }

    SECTION("Throw runtime_error. final_state_density > 1") {
        num_of_levels = 2;
        num_of_states = 10;
        alphabet_sizes = {3, 4};
        states_trans_ratio_per_symbol = 0.5;
        final_state_density = static_cast<double>(1.1);

        CHECK_THROWS_AS(mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density), std::runtime_error);
    }

    SECTION("Throw runtime_error. num_of_levels != length of alphabet_sizes vector") {
        num_of_levels = 2;
        num_of_states = 10;
        alphabet_sizes = {2, 3, 4};
        states_trans_ratio_per_symbol = 0.5;
        final_state_density = 0.5;

        CHECK_THROWS_AS(mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density), std::runtime_error);
    }
}

TEST_CASE("Padding closure", "[mata::ext::padding_closure]") {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    Nfa aut(4);

    aut.initial = { 0 };
    aut.final = { 3 };
    aut.delta.add(3, 0, 3);
    aut.delta.add(0, 0, 1);
    aut.delta.add(1, 1, 2);
    aut.delta.add(2, 0, 3);

    REQUIRE(!aut.is_in_lang({0, 1}));
    std::cout << aut.print_to_dot() << std::endl;
    mata::ext::padding_closure(aut, 0);
    std::cout << aut.print_to_dot() << std::endl;
    REQUIRE(aut.is_in_lang({0, 1}));
}
