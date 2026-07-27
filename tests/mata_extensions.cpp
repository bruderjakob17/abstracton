#include <catch2/catch_test_macros.hpp>
#include <mata/nft/nft.hh>
#include <mata/parser/re2parser.hh>
#include <abstracton/mata_extensions.hpp>
#include <algorithm>
#include <abstracton/utils/utils.hpp>
#include <mata/nfa/builder.hh>
#include <mata/nft/builder.hh>

TEST_CASE( "Insert NFA into NFA", "[insert_nfa_between]" ) {
    using namespace mata;
    using namespace mata::nfa;

    Nfa aut(4, {0}, {3});
    aut.delta.add(0, 'a', 1);
    aut.delta.add(2, 'c', 3);

    Nfa inserted(3, {0}, {2});
    inserted.delta.add(0, 'b', 1);
    inserted.delta.add(1, 'b', 2);
    inserted.delta.add(0, 'd', 2);

    insert_nfa_between(aut, 1, 2, inserted);

    REQUIRE(aut.get_words(10) == std::set<Word> {
        {'a', 'b', 'b', 'c'},
        {'a', 'd', 'c'}
    });
}

TEST_CASE( "Insert NFT into NFT", "[insert_nft_between]" ) {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    Nfa abcd = mata::nfa::builder::create_from_regex("abcd");
    Nft aut = mata::nft::builder::from_nfa_with_levels_advancing(abcd, 2);

    Nfa ef = mata::nfa::builder::create_from_regex("ef|fe");
    Nft inserted = mata::nft::builder::from_nfa_with_levels_advancing(ef, 2);

    StateSet q1 = aut.read_word({'a'});
    StateSet q3 = aut.read_word({'a', 'b', 'c'});
    REQUIRE(q1.size() == 1);
    REQUIRE(q3.size() == 1);
    insert_nft_between(aut, q1.front(), q3.front(), inserted);

    REQUIRE(aut.get_words(10) == std::set<Word> {
        {'a', 'b', 'c', 'd'},
        {'a', 'f', 'e', 'd'},
        {'a', 'e', 'f', 'd'}
    });
    StateSet qaf = aut.read_word({'a', 'f'});
    REQUIRE(qaf.size() == 1);
    REQUIRE(aut.levels[qaf.front()] == 0);

    Nft inserted2 = mata::nft::builder::from_nfa_with_levels_advancing(mata::nfa::builder::create_from_regex("xy"), 2);
    insert_nft_between(aut, aut.read_word({'a', 'b'}).front(), aut.read_word({'a', 'b', 'c', 'd'}).front(), inserted2);
    REQUIRE(aut.levels[aut.read_word({'a', 'b', 'x'}).front()] == 1);
}

TEST_CASE( "Identity on Alphabet correct", "[create_identity]" ) {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    EnumAlphabet alphabet{'0', '1'};

    Nft id {create_identity(alphabet)};

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

TEST_CASE("Nft complement", "[mata::ext::complement]") {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    Nft aut = Nft::with_levels(2, 4, { 0 }, { 3 });
    aut.add_transition(0, {0, 0}, 1);
    aut.add_transition(0, {1, 1}, 2);
    aut.add_transition(2, {1, 1}, 1);
    aut.add_transition(1, {1, 1}, 2);
    aut.add_transition(1, {0, 1}, 1);
    aut.add_transition(2, {0, 1}, 2);
    aut.add_transition(1, {0, 0}, 3);
    aut.add_transition(1, {0, 0}, 0); // non-deterministic
    aut.add_transition(2, {0, 0}, 3);

    SECTION("Min during det") {
        Nft comp {mata::ext::complement(aut, nullptr, true)};

        for (int k { 0 }; k <= 3; ++k) {
            // create vector filled with 2k 2s
            std::vector<unsigned int> tuple_bounds(2 * k, 2);
            std::cout << "[";
            for (unsigned int i : tuple_bounds) {
                std::cout << i << ", ";
            }
            std::cout << "]\n";
            for (auto t : BoundedTuples(tuple_bounds)) {
                std::cout << "[";
                for (unsigned int i : t) {
                    std::cout << i << ", ";
                }
                std::cout << "]\n";
                REQUIRE(aut.is_in_lang(t) != comp.is_in_lang(t));
            }
        }
    }

    SECTION("Only det") {
        Nft comp {mata::ext::complement(aut, nullptr, false)};

        for (int k { 0 }; k <= 3; ++k) {
            // create vector filled with 2k 2s
            std::vector<unsigned int> tuple_bounds(2 * k, 2);
            std::cout << "[";
            for (unsigned int i : tuple_bounds) {
                std::cout << i << ", ";
            }
            std::cout << "]\n";
            for (auto t : BoundedTuples(tuple_bounds)) {
                std::cout << "[";
                for (unsigned int i : t) {
                    std::cout << i << ", ";
                }
                std::cout << "]\n";
                REQUIRE(aut.is_in_lang(t) != comp.is_in_lang(t));
            }
        }
    }

    SECTION("empty nft") {
        Nft empty = mata::nft::Nft::with_levels(2, 0, {}, {});

        std::cout << "complementing empty nft:" << std::endl;
        std::cout << empty.print_to_dot() << std::endl;

        EnumAlphabet alph {0, 1};
        AlphabetLevels alph_levels({&alph}, AlphabetLevels::Mode::Global);

        Nft univ1 {mata::ext::complement(empty, &alph_levels, true)};
        Nft univ2 {mata::ext::complement(empty, &alph_levels, false)};

        std::cout << "result (with minimization):" << std::endl;
        std::cout << univ1.print_to_dot() << std::endl;
        std::cout << "result (without minimization):" << std::endl;
        std::cout << univ2.print_to_dot() << std::endl;

        // check univ1
        REQUIRE(univ1.is_in_lang(Word {}));

        REQUIRE(univ1.is_in_lang({0, 0}));
        REQUIRE(univ1.is_in_lang({0, 1}));
        REQUIRE(univ1.is_in_lang({1, 0}));
        REQUIRE(univ1.is_in_lang({1, 1}));
        REQUIRE(!univ1.is_in_lang({0, 2}));
        REQUIRE(!univ1.is_in_lang({2, 0}));
        REQUIRE(!univ1.is_in_lang({2, 2}));

        REQUIRE(!univ1.is_in_lang({0, 1, 0, 2}));
        REQUIRE(univ1.is_in_lang({0, 1, 0, 1}));

        // check univ2
        REQUIRE(univ2.is_in_lang(Word {}));

        REQUIRE(univ2.is_in_lang({0, 0}));
        REQUIRE(univ2.is_in_lang({0, 1}));
        REQUIRE(univ2.is_in_lang({1, 0}));
        REQUIRE(univ2.is_in_lang({1, 1}));
        REQUIRE(!univ2.is_in_lang({0, 2}));
        REQUIRE(!univ2.is_in_lang({2, 0}));
        REQUIRE(!univ2.is_in_lang({2, 2}));

        REQUIRE(!univ2.is_in_lang({0, 1, 0, 2}));
        REQUIRE(univ2.is_in_lang({0, 1, 0, 1}));
    }
}

TEST_CASE("Create Sigma Star NFT", "[mata::ext::create_sigma_star_nft]") {
    using namespace mata;
    using namespace mata::nft;

    EnumAlphabet ab_alphabet = {'a', 'b'};
    EnumAlphabet ac_alphabet = {'a', 'c'};

    AlphabetLevels ab_alphabet_levels(&ab_alphabet);
    AlphabetLevels ab_alphabet_levels_multilevel({&ab_alphabet}, AlphabetLevels::Mode::MultiLevel);
    AlphabetLevels ab_ac_alphabet_levels({&ab_alphabet, &ac_alphabet});
    AlphabetLevels ab_ac_ab_alphabet_levels({&ab_alphabet, &ac_alphabet, &ab_alphabet});

    // TODO: mata is buggy with 0 tapes...
    // SECTION("0 tapes") {
    //     Nft nft0 = mata::ext::create_sigma_star_nft(0);
    //
    //     REQUIRE(nft0.is_in_lang({}));
    //     REQUIRE(!nft0.is_in_lang({'a'}));
    // }
    // TODO: mata does not seem to handle DONT_CARE symbols in is_in_lang...
    // SECTION("1 tape, no alphabet") {
    //     Nft nft = mata::ext::create_sigma_star_nft(1);
    //     REQUIRE(nft.is_in_lang({}));
    //     REQUIRE(nft.is_in_lang({'a'}));
    //     REQUIRE(nft.is_in_lang({'b'}));
    //     REQUIRE(nft.is_in_lang({'a', 'b'}));
    // }
    SECTION("1 tape, generic alphabet") {
        Nft nft = mata::ext::create_sigma_star_nft(1, &ab_alphabet_levels);
        REQUIRE(nft.is_in_lang(Word {}));
        REQUIRE(nft.is_in_lang({'a'}));
        REQUIRE(nft.is_in_lang({'b'}));
        REQUIRE(!nft.is_in_lang({'c'}));
        REQUIRE(nft.is_in_lang({'a', 'b'}));
        REQUIRE(!nft.is_in_lang({'a', 'c'}));
    }
    SECTION("1 tape, specific alphabet") {
        Nft nft = mata::ext::create_sigma_star_nft(1, &ab_alphabet_levels_multilevel);
        REQUIRE(nft.is_in_lang(Word {}));
        REQUIRE(nft.is_in_lang({'a'}));
        REQUIRE(nft.is_in_lang({'b'}));
        REQUIRE(!nft.is_in_lang({'c'}));
        REQUIRE(nft.is_in_lang({'a', 'b'}));
        REQUIRE(!nft.is_in_lang({'a', 'c'}));
    }
    SECTION("2 tapes, generic alphabet") {
        Nft nft = mata::ext::create_sigma_star_nft(2, &ab_alphabet_levels);
        REQUIRE(nft.is_in_lang(Word {}));
        // REQUIRE(!nft.is_in_lang({'a'}));
        // REQUIRE(!nft.is_in_lang({'b'}));
        // REQUIRE(!nft.is_in_lang({'c'}));
        REQUIRE(nft.is_in_lang({'a', 'b'}));
        REQUIRE(!nft.is_in_lang({'a', 'c'}));
        // REQUIRE(!nft.is_in_lang({'a', 'b', 'b'}));
        REQUIRE(nft.is_in_lang({'a', 'b', 'b', 'b'}));
    }
    SECTION("2 tapes, specific alphabets") {
        Nft nft = mata::ext::create_sigma_star_nft(2, &ab_ac_alphabet_levels);

        REQUIRE(nft.is_in_lang(Word {}));
        // REQUIRE(!nft.is_in_lang({'a'}));

        REQUIRE(nft.is_in_lang({'a', 'a'}));
        REQUIRE(nft.is_in_lang({'b', 'a'}));
        REQUIRE(nft.is_in_lang({'a', 'c'}));
        REQUIRE(nft.is_in_lang({'b', 'c'}));

        REQUIRE(!nft.is_in_lang({'c', 'a'}));
        REQUIRE(!nft.is_in_lang({'a', 'b'}));
        REQUIRE(!nft.is_in_lang({'c', 'b'}));

        REQUIRE(!nft.is_in_lang({'a', 'b', 'a', 'c'}));
        REQUIRE(nft.is_in_lang({'a', 'a', 'b', 'c'}));
    }
    SECTION("3 tapes, generic alphabet, compare with complement of EMPTYSET") {
        Nft nft = mata::ext::create_sigma_star_nft(3, &ab_alphabet_levels);

        Nft empty = Nft::with_levels(3);
        Nft univ = mata::ext::complement(empty, &ab_alphabet_levels);

        std::cout << "create_sigma_star_nft output:\n";
        std::cout << nft.print_to_dot(true) << std::endl;
        std::cout << "complementing empty nft:\n";
        std::cout << univ.print_to_dot(true) << std::endl;

        REQUIRE(mata::nft::are_equivalent(nft, univ));
    }
    SECTION("3 tapes, specific alphabets, compare with complement of EMPTYSET") {
        Nft nft = mata::ext::create_sigma_star_nft(3, &ab_ac_ab_alphabet_levels);

        Nft empty = Nft::with_levels(3);
        Nft univ = mata::ext::complement(empty, &ab_ac_ab_alphabet_levels);

        std::cout << "create_sigma_star_nft output:\n";
        std::cout << nft.print_to_dot(true) << std::endl;
        std::cout << "complementing empty nft:\n";
        std::cout << univ.print_to_dot(true) << std::endl;

        REQUIRE(mata::nft::are_equivalent(nft, univ));
    }
}

TEST_CASE("Complement of empty NFT is Sigma Star") {
    using namespace mata;
    using namespace mata::nft;

    SECTION("2 tapes, 2 symbols") {
        Nft empty = mata::nft::Nft::with_levels(2, 0, {}, {});

        std::cout << "complementing empty nft:" << std::endl;
        std::cout << empty.print_to_dot() << std::endl;

        EnumAlphabet alph {0, 1};
        AlphabetLevels alph_levels(&alph);

        Nft univ1 {mata::ext::complement(empty, &alph_levels, true)};
        Nft univ2 {mata::ext::complement(empty, &alph_levels, false)};

        std::cout << "result (with minimization):" << std::endl;
        std::cout << univ1.print_to_dot() << std::endl;
        std::cout << "result (without minimization):" << std::endl;
        std::cout << univ2.print_to_dot() << std::endl;

        // check univ1
        REQUIRE(mata::nft::are_equivalent(univ1, mata::ext::create_sigma_star_nft(2, &alph_levels)));

        // check univ2
        REQUIRE(mata::nft::are_equivalent(univ2, mata::ext::create_sigma_star_nft(2, &alph_levels)));
    }
}

TEST_CASE("Universality for length-preserving NFTs using antichains") {
    using namespace mata;

    auto print_run = [](mata::nft::Run cex) -> void {
        std::cout << "Run:\n";
        std::cout << '\t' << vec_to_string(cex.word) << std::endl;
        std::cout << "Trace in NFT:\n";
        std::cout << '\t' << vec_to_string(cex.path) << std::endl;
    };

    // define a few sample nfts
    mata::EnumAlphabet ab_alph {'a', 'b'};
    AlphabetLevels ab_alphabet_levels(&ab_alph);
    std::vector<mata::Alphabet*> ab_alphs = {&ab_alph, &ab_alph};
    mata::nft::Nft univ = mata::nft::Nft::with_levels(2, 2, {0}, {0});
    //univ.final.insert(0); //TODO why does mata not automatically add 0 as final state in with_levels()?
    univ.delta.add(0, 'a', 1);
    univ.delta.add(0, 'b', 1);
    univ.delta.add(1, 'a', 0);
    univ.delta.add(1, 'b', 0);
    univ.levels[0] = 0;
    univ.levels[1] = 1;

    mata::EnumAlphabet bc_alph {'b', 'c'};
    AlphabetLevels ab_bc_alphabet_levels({&ab_alph, &bc_alph});
    mata::nft::Nft univ2 = mata::ext::create_sigma_star_nft(2, &ab_bc_alphabet_levels);

    std::cout << univ.print_to_dot(true) << std::endl;
    std::cout << univ2.print_to_dot(true) << std::endl;

    SECTION("modification of mata::nft::algorithms::is_universal_antichains: [mata::ext::is_universal_antichains]") {
        bool is_univ;
        mata::nft::Run cex;

        is_univ = mata::ext::is_universal_antichains(univ, &ab_alphabet_levels, &cex);
        print_run(cex);
        REQUIRE(is_univ);

        is_univ = mata::ext::is_universal_antichains(univ2, &ab_bc_alphabet_levels, &cex);
        print_run(cex);
        REQUIRE(is_univ);

        is_univ = mata::ext::is_universal_antichains(univ2, &ab_alphabet_levels, &cex);
        print_run(cex);
        REQUIRE(!is_univ);
    }

    SECTION("using mata::nft::algorithms::is_included_antichains in combination with create_sigma_star_nft: [mata::ext::is_universal_antichains_by_inclusion]") {
        bool is_univ;
        mata::nft::Run cex;

        is_univ = mata::ext::is_universal_antichains_by_inclusion(univ, &ab_alphabet_levels, &cex);
        print_run(cex);
        REQUIRE(is_univ);

        is_univ = mata::ext::is_universal_antichains_by_inclusion(univ2, &ab_bc_alphabet_levels, &cex);
        print_run(cex);
        REQUIRE(is_univ);

        is_univ = mata::ext::is_universal_antichains_by_inclusion(univ2, &ab_alphabet_levels, &cex);
        print_run(cex);
        REQUIRE(!is_univ);
    }

    SECTION("using lazy exploration") {
        bool is_univ;
        mata::nft::Run cex;

        is_univ = mata::ext::is_universal_lazy(univ, &ab_alphabet_levels, &cex);
        print_run(cex);
        REQUIRE(is_univ);

        is_univ = mata::ext::is_universal_lazy(univ2, &ab_bc_alphabet_levels, &cex);
        print_run(cex);
        REQUIRE(is_univ);

        is_univ = mata::ext::is_universal_lazy(univ2, &ab_alphabet_levels, &cex);
        print_run(cex);
        REQUIRE(!is_univ);
    }
}

TEST_CASE("Insert tapes", "[mata::ext::insert_tapes]") {
    using namespace mata;
    using namespace mata::nft;

    EnumAlphabet ab_alph {'a', 'b'};
    EnumAlphabet bc_alph {'b', 'c'};
    EnumAlphabet cd_alph {'c', 'd'};
    EnumAlphabet ef_alph {'e', 'f'};
    EnumAlphabet x_alph {'x'};
    EnumAlphabet yz_alph {'y', 'z'};

    SECTION("Simple example") {
        AlphabetLevels x_yz_alphabet_levels({&x_alph, &yz_alph});
        Nft aut = Nft::with_levels(2, 4, {0}, {2, 3}, &x_yz_alphabet_levels);
        aut.levels[0] = 0;
        aut.levels[1] = 1;
        aut.levels[2] = 0;
        aut.levels[3] = 0;
        aut.delta.add(0, 'x', 1);
        aut.delta.add(1, 'y', {0, 2});
        aut.delta.add(1, 'z', 3);
        std::cout << aut.print_to_dot(true) << std::endl;

        assert((aut.get_words(4) == std::set<std::vector<Symbol>>{{'x', 'y'}, {'x', 'z'}, {'x', 'y', 'x', 'y'}, {'x', 'y', 'x', 'z'}}));

        AlphabetLevels ab_cd_ef_alphabet_levels({&ab_alph, &cd_alph, &ef_alph});
        Nft aut_inserted = mata::ext::insert_tapes(aut, {0, 2, 4}, ab_cd_ef_alphabet_levels.alphabets);
        std::cout << aut_inserted.print_to_dot(true) << std::endl;

        REQUIRE(aut_inserted.get_words(5) == std::set<std::vector<Symbol>>{
            {'a', 'x', 'c', 'y', 'e'},
            {'a', 'x', 'c', 'y', 'f'},
            {'a', 'x', 'd', 'y', 'e'},
            {'a', 'x', 'd', 'y', 'f'},
            {'b', 'x', 'c', 'y', 'e'},
            {'b', 'x', 'c', 'y', 'f'},
            {'b', 'x', 'd', 'y', 'e'},
            {'b', 'x', 'd', 'y', 'f'},
            {'a', 'x', 'c', 'z', 'e'},
            {'a', 'x', 'c', 'z', 'f'},
            {'a', 'x', 'd', 'z', 'e'},
            {'a', 'x', 'd', 'z', 'f'},
            {'b', 'x', 'c', 'z', 'e'},
            {'b', 'x', 'c', 'z', 'f'},
            {'b', 'x', 'd', 'z', 'e'},
            {'b', 'x', 'd', 'z', 'f'}
        });

        // project back; result must accept same language as the nft we started with

        Nft aut_back_projection = mata::nft::project_out(aut_inserted, {0, 2, 4});
        std::cout << aut_back_projection.print_to_dot(true) << std::endl;

        REQUIRE(mata::nft::are_equivalent(aut, aut_back_projection));
    }

    // TODO: multiple initial states, final initial states, ...
    // TODO later, for relational product, test that sigma star products are equivalent to sigma star nft

    SECTION("Sigma Star") {
        AlphabetLevels ab_bc_alphabet_levels({&ab_alph, &bc_alph});
        AlphabetLevels x_yz_alphabet_levels({&x_alph, &yz_alph});
        Nft ab_bc_aut = mata::ext::create_sigma_star_nft(2, &ab_bc_alphabet_levels);
        Nft x_yz_aut = mata::ext::create_sigma_star_nft(2, &x_yz_alphabet_levels);

        std::cout << "{a, b} x {b, c}:\n" << ab_bc_aut.print_to_dot(true) << std::endl;
        std::cout << "{x} x {y, z}:\n" << x_yz_aut.print_to_dot(true) << std::endl;

        Nft ab_x_bc_yz_aut1 = mata::ext::insert_tapes(ab_bc_aut, {1, 3}, x_yz_alphabet_levels.alphabets);
        std::cout << "inserting tapes {x}, {y, z} in first automaton:\n" <<ab_x_bc_yz_aut1.print_to_dot(true) << std::endl;
        Nft ab_x_bc_yz_aut2 = mata::ext::insert_tapes(x_yz_aut, {0, 2}, ab_bc_alphabet_levels.alphabets);
        std::cout << "inserting tapes {a, b}, {b, c} in second automaton:\n" <<ab_x_bc_yz_aut2.print_to_dot(true) << std::endl;

        REQUIRE(mata::nft::are_equivalent(ab_x_bc_yz_aut1, ab_x_bc_yz_aut2));
    }
}

TEST_CASE("Relational (length-preserving) product", "[mata::ext::relational_product_length_preserving]") {
    using namespace mata;
    using namespace mata::nft;

    // SECTION("Empty Product") {
    //     Nft e = mata::ext::relational_product_length_preserving({});
    //     REQUIRE(e.get_words() == std::set<std::vector<Symbol>>{{}});
    // }
    SECTION("Product of one NFT") {
        Nft aut = mata::ext::builder::create_random_nft_tabakov_vardi(2, 6, {3, 3}, 5.0, 0.5, std::make_optional(5));
        Nft prod = mata::ext::relational_product_length_preserving({aut});
        REQUIRE(mata::nft::are_equivalent(aut, prod));
    }
    SECTION("Product of two NFTs") {
        // {(ab, aa), (ab, bb)} x {(d, c), (cc, dd)} = {(ab, aa, cc, dd), (ab, bb, cc, dd)}

        Nft aut1 = Nft::with_levels(2, 1, {0});
        aut1.final.insert(aut1.insert_word_by_levels(0, std::vector<Word>{{'a', 'b'}, {'a', 'a'}}));
        aut1.final.insert(aut1.insert_word_by_levels(0, std::vector<Word>{{'a', 'b'}, {'b', 'b'}}));
        std::cout << aut1.print_to_dot(true) << std::endl;

        Nft aut2 = Nft::with_levels(2, 1, {0});
        aut2.final.insert(aut2.insert_word_by_levels(0, std::vector<Word>{{'d'}, {'c'}}));
        aut2.final.insert(aut2.insert_word_by_levels(0, std::vector<Word>{{'c', 'c'}, {'d', 'd'}}));
        std::cout << aut2.print_to_dot(true) << std::endl;

        Nft prod = mata::ext::relational_product_length_preserving({aut1, aut2});
        std::cout << prod.print_to_dot(true) << std::endl;

        Nft expected = Nft::with_levels(4, 1, {0});
        expected.final.insert(expected.insert_word_by_levels(0, std::vector<Word>{{'a', 'b'}, {'a', 'a'}, {'c', 'c'}, {'d', 'd'}}));
        expected.final.insert(expected.insert_word_by_levels(0, std::vector<Word>{{'a', 'b'}, {'b', 'b'}, {'c', 'c'}, {'d', 'd'}}));
        std::cout << expected.print_to_dot(true) << std::endl;

        REQUIRE(are_equivalent(prod, expected));

        // also test version with dont_care
        Nft prod_dont_care = mata::ext::relational_product_length_preserving_dont_care({aut1, aut2});
        REQUIRE(are_equivalent(prod, prod_dont_care));
    }
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
        num_of_levels = 2;
        num_of_states = 0;
        alphabet_sizes = {0, 0};
        states_trans_ratio_per_symbol = 0;
        final_state_density = 0;

        mata::nft::Nft nft = mata::ext::builder::create_random_nft_tabakov_vardi(num_of_levels, num_of_states, alphabet_sizes, states_trans_ratio_per_symbol, final_state_density);
        CHECK(nft.levels.num_of_levels == num_of_levels);
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

    SECTION("Same seed results in same NFT.") {
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
