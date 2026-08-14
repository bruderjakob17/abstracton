#include <catch2/catch_test_macros.hpp>
#include <mata/alphabet.hh>
#include <mata/nft/nft.hh>
#include <mata/parser/re2parser.hh>
#include <abstracton/bddlike/bddlike_nft.hpp>
#include <abstracton/mata_extensions.hpp>
#include <abstracton/utils/utils.hpp>

TEST_CASE( "DefaultVecAlphabet", "[DefaultVecAlphabet]" ) {
    using namespace mata::ext::bddlike;

    DefaultVecAlphabet alph = DefaultVecAlphabet(3);
    CHECK(alph.dimension == 3);
    CHECK(alph.translate_symbol({0, 15, 1}) == std::vector<mata::Symbol>{0, 15, 1});
    CHECK(alph.reverse_translate_symbol({0, 15, 1}) == std::vector<mata::Symbol>{0, 15, 1});
    CHECK(alph.print({0, 15, 1}) == std::string("[0, 15, 1]"));
}

TEST_CASE( "SimpleVecAlphabet", "[SimpleVecAlphabet]" ) {
    using namespace mata;
    using namespace mata::ext::bddlike;
    using namespace std;

    SECTION("EnumAlphabet") {
        EnumAlphabet alphabet = {'#', 'a', 'b'};

        // WARNING EnumAlphabet is weird... 'a' = 97, so to access it, one must use the string "97":
        SimpleVecAlphabet alph = SimpleVecAlphabet(make_shared<EnumAlphabet>(alphabet));
        CHECK(alph.dimension == 1);
        CHECK(alph.translate_symbol("97") == vector<Symbol>{'a'});
        CHECK(alph.reverse_translate_symbol({'a'}) == string("97"));
        CHECK(alph.print({'a'}) == string("97"));
    }

    SECTION("OnTheFlyAlphabet") {
        OnTheFlyAlphabet alphabet{vector<string>{"ab", "c", "d", "ef", "bab"}};

        SimpleVecAlphabet alph = SimpleVecAlphabet(make_shared<OnTheFlyAlphabet>(alphabet));
        CHECK(alph.dimension == 1);
        CHECK(alph.translate_symbol("ef") == vector<Symbol>{3});
        CHECK(alph.reverse_translate_symbol({3}) == string("ef"));
        CHECK(alph.print({3}) == string("ef"));
    }
}

TEST_CASE( "PowersetVecAlphabet", "[PowersetVecAlphabet]" ) {
    using namespace mata;
    using namespace mata::ext::bddlike;
    using namespace std;

    OnTheFlyAlphabet alphabet{vector<string>{"ab", "c", "d"}};

    PowersetVecAlphabet alph = PowersetVecAlphabet(make_shared<OnTheFlyAlphabet>(alphabet));
    CHECK(alph.dimension == 3);
    CHECK(alph.translate_symbol({"ab", "c"}) == vector<Symbol>{1, 1, 0});
    CHECK(alph.reverse_translate_symbol({1, 1, 0}) == vector<string>{"ab", "c"});
    CHECK(alph.print({1, 1, 0}) == "{ab, c}");
    CHECK(alph.translate_word({}) == vector<vector<Symbol>>{});
    CHECK(alph.translate_word({{"ab", "c"}}) == vector<vector<Symbol>>{{1, 1, 0}});
    CHECK(alph.translate_word({{"ab", "c"}, {"ab", "c"}, {}, {"c", "d"}, {"ab", "c", "d"}}) ==
            vector<vector<Symbol>>{{1, 1, 0}, {1, 1, 0}, {0, 0, 0}, {0, 1, 1}, {1, 1, 1}});
    CHECK(alph.reverse_translate_word({{1, 1, 0}, {1, 1, 0}, {0, 0, 0}, {0, 1, 1}, {1, 1, 1}}) ==
            vector<vector<string>>{{"ab", "c"}, {"ab", "c"}, {}, {"c", "d"}, {"ab", "c", "d"}});
}

TEST_CASE( "BaseMSDVecAlphabet", "[BaseMSDVecAlphabet]") {
    using namespace mata;
    using namespace mata::ext::bddlike;
    using namespace std;

    BaseMSDVecAlphabet alph = BaseMSDVecAlphabet(2, 4);
    CHECK(alph.dimension == 4);

    // 5
    CHECK(alph.translate_symbol(5) == vector<Symbol>{0, 1, 0, 1});
    CHECK(alph.reverse_translate_symbol({0, 1, 0, 1}) == 5);
    CHECK(alph.print({0, 1, 0, 1}) == "5");

    // 11
    CHECK(alph.translate_symbol(11) == vector<Symbol>{1, 0, 1, 1});
    CHECK(alph.reverse_translate_symbol({1, 0, 1, 1}) == 11);
    CHECK(alph.print({1, 0, 1, 1}) == "11");

    // 0
    CHECK(alph.translate_symbol(0) == vector<Symbol>{0, 0, 0, 0});
    CHECK(alph.reverse_translate_symbol({0, 0, 0, 0}) == 0);
    CHECK(alph.print({0, 0, 0, 0}) == "0");

    // 16 (too large symbol)
    CHECK_THROWS(alph.translate_symbol(16));
}

TEST_CASE( "BaseLSDVecAlphabet", "[BaseLSDVecAlphabet]") {
    using namespace mata;
    using namespace mata::ext::bddlike;
    using namespace std;

    BaseLSDVecAlphabet alph = BaseLSDVecAlphabet(2, 4);
    CHECK(alph.dimension == 4);

    // 5
    CHECK(alph.translate_symbol(5) == vector<Symbol>{1, 0, 1, 0});
    CHECK(alph.reverse_translate_symbol({1, 0, 1, 0}) == 5);
    CHECK(alph.print({1, 0, 1, 0}) == "5");

    // 11
    CHECK(alph.translate_symbol(11) == vector<Symbol>{1, 1, 0, 1});
    CHECK(alph.reverse_translate_symbol({1, 1, 0, 1}) == 11);
    CHECK(alph.print({1, 1, 0, 1}) == "11");

    // 0
    CHECK(alph.translate_symbol(0) == vector<Symbol>{0, 0, 0, 0});
    CHECK(alph.reverse_translate_symbol({0, 0, 0, 0}) == 0);
    CHECK(alph.print({0, 0, 0, 0}) == "0");

    // 16 (too large symbol)
    CHECK_THROWS(alph.translate_symbol(16));
}

TEST_CASE( "bddlike_nft default alphabet", "[bddlike_nft]" ) {
    using namespace mata::ext::bddlike;

    BDDlikeNft aut = BDDlikeNft::with_alphabet_sizes({2, 3});
    CHECK(aut.levels.num_of_levels == 5);
}

TEST_CASE( "BDDlikeNft compose function", "[BDDlikeNft::compose]" ) {
    using namespace mata::ext::bddlike;

    BDDlikeNft aut1 = BDDlikeNft::with_alphabet_sizes({2, 2, 2}, 2, {0}, {1});
    BDDlikeNft aut2 = BDDlikeNft::with_alphabet_sizes({2, 2, 2}, 2, {0}, {1});

    aut1.insert_word(0, {0, 0, 1, 2, 3, 4}, 1);
    aut2.insert_word(0, {5, 6, 1, 2, 7, 8}, 1);
    aut2.insert_word(0, {0, 0, 1, 0, 3, 4}, 1);

    BDDlikeNft comp = mata::ext::bddlike::compose(aut1, aut2, {1}, {1}, true);

    REQUIRE(
        comp.get_words(20) == std::set<mata::Word> {
            {0, 0, 5, 6, 3, 4, 7, 8}
        }
    );
}

TEST_CASE( "BDDlikeNft project_to", "[BDDlikeNft::project_to]" ) {
    using namespace mata::ext::bddlike;

    BDDlikeNft aut = BDDlikeNft::with_alphabet_sizes({2, 2, 2}, 2, {0}, {1});

    aut.insert_word(0, {0, 0, 1, 2, 3, 4}, 1);
    aut.insert_word(0, {1, 0, 3, 3, 4, 1}, 1);

    std::vector<BDDlikeNft> results{};
    for (mata::nft::Level i = 0; i < 3; ++i) {
        results.push_back(mata::ext::bddlike::project_to(aut, {i}));
    }
    results.push_back(mata::ext::bddlike::project_to(aut, {0, 2}));

    REQUIRE(
        results[0].get_words(20) == std::set<mata::Word> {
            {0, 0},
            {1, 0}
        }
    );
    REQUIRE(
        results[1].get_words(20) == std::set<mata::Word> {
            {1, 2},
            {3, 3}
        }
    );
    REQUIRE(
        results[2].get_words(20) == std::set<mata::Word> {
            {3, 4},
            {4, 1}
        }
    );
    REQUIRE(
        results[3].get_words(20) == std::set<mata::Word> {
            {0, 0, 3, 4},
            {1, 0, 4, 1}
        }
    );
}

TEST_CASE( "BDDlikeNft complement", "[BDDlikeNft::complement]" ) {
    using namespace mata::ext::bddlike;

    mata::OnTheFlyAlphabet alph(std::vector<std::string>{"ab", "cde"});
    AlphabetVecAlphabet vec_alph(std::make_shared<mata::OnTheFlyAlphabet>(alph), 2);
    std::vector<std::shared_ptr<VecAlphabetPrinter>> vec_alph_vec{};
    vec_alph_vec.emplace_back(std::make_shared<AlphabetVecAlphabet>(vec_alph));
    vec_alph_vec.emplace_back(std::make_shared<AlphabetVecAlphabet>(vec_alph));

    BDDlikeNft aut = BDDlikeNft::with_alphabet_sizes({2, 2}, 2, {0}, {1}, std::make_optional<std::vector<std::shared_ptr<VecAlphabetPrinter>>>(vec_alph_vec));

    mata::nft::State q1 = aut.insert_word(0, vec_alph.translate_symbol({"ab", "ab"}));
    aut.insert_word(q1, vec_alph.translate_symbol({"cde", "ab"}), 1);

    std::cout << "automaton:\n";
    std::cout << aut.print_to_dot() << std::endl;

    BDDlikeNft result = complement(aut);

    std::cout << "complement:\n";
    std::cout << result.print_to_dot() << std::endl;

    std::vector<mata::Symbol> p1 = vec_alph.translate_symbol({"ab", "ab"});
    std::vector<mata::Symbol> p2 = vec_alph.translate_symbol({"cde", "ab"});
    auto w1 = flatten(std::vector<std::vector<mata::Symbol>>{p1, p2});
    auto w2 = flatten(std::vector<std::vector<mata::Symbol>>{p1, p1});

    REQUIRE(!result.get_words(4).contains(w1));
    REQUIRE(result.get_words(4).contains(w2));
}

TEST_CASE( "BDDlikeNft inclusion", "[BDDlikeNft::is_included_lazy, BDDlikeNft::is_included_antichains]" ) {
    using namespace mata::ext::bddlike;

    mata::OnTheFlyAlphabet alph(std::vector<std::string>{"req", "syn", "ack"});
    SimpleVecAlphabet vec_alph_0(std::make_shared<mata::OnTheFlyAlphabet>(alph));
    BoundedDefaultVecAlphabet vec_alph_1(2, 3);

    std::vector<std::shared_ptr<VecAlphabetPrinter>> vec_alph_vec{};
    vec_alph_vec.emplace_back(std::make_shared<SimpleVecAlphabet>(vec_alph_0));
    vec_alph_vec.emplace_back(std::make_shared<BoundedDefaultVecAlphabet>(vec_alph_1));

    BDDlikeNft aut_0 = BDDlikeNft::with_alphabet_sizes({1, 3}, 2, {0}, {1}, std::make_optional<std::vector<std::shared_ptr<VecAlphabetPrinter>>>(vec_alph_vec));
    BDDlikeNft aut_1 = BDDlikeNft::with_alphabet_sizes({1, 3}, 2, {0}, {1}, std::make_optional<std::vector<std::shared_ptr<VecAlphabetPrinter>>>(vec_alph_vec));
    BDDlikeNft aut_2 = BDDlikeNft::with_alphabet_sizes({1, 3}, 2, {0}, {1}, std::make_optional<std::vector<std::shared_ptr<VecAlphabetPrinter>>>(vec_alph_vec));

    insert_word(aut_0, 0, std::vector<std::string>{"syn", "ack", "req"}, std::vector<std::vector<mata::Symbol>>{{0, 0, 0}, {0, 1, 1}, {0, 1, 1}}, 1);
    insert_word(aut_1, 0, std::vector<std::string>{"syn", "ack", "req"}, std::vector<std::vector<mata::Symbol>>{{0, 0, 0}, {0, 1, 1}, {0, 1, 1}}, 1);
    insert_word(aut_2, 0, std::vector<std::string>{"syn", "ack", "req"}, std::vector<std::vector<mata::Symbol>>{{0, 0, 0}, {0, 1, 1}, {0, 1, 1}}, 1);
    insert_word(aut_1, 0, std::vector<std::string>{"syn", "ack", "req"}, std::vector<std::vector<mata::Symbol>>{{0, 0, 0}, {0, 0, 1}, {0, 1, 1}}, 1);
    insert_word(aut_2, 0, std::vector<std::string>{"syn", "ack", "ack"}, std::vector<std::vector<mata::Symbol>>{{0, 0, 0}, {0, 1, 1}, {0, 1, 1}}, 1);

    // aut_0 contained in aut_1 and aut_2
    REQUIRE(is_included_lazy(aut_0, aut_1, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(is_included_lazy(aut_0, aut_1, nullptr, logging::VerbosityLevel::DEBUGV, false));
    REQUIRE(is_included_lazy(aut_0, aut_2, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(is_included_lazy(aut_0, aut_2, nullptr, logging::VerbosityLevel::DEBUGV, false));
    REQUIRE(is_included_antichains(aut_0, aut_1, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(is_included_antichains(aut_0, aut_1, nullptr, logging::VerbosityLevel::DEBUGV, false));
    REQUIRE(is_included_antichains(aut_0, aut_2, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(is_included_antichains(aut_0, aut_2, nullptr, logging::VerbosityLevel::DEBUGV, false));

    // aut_1 is not contained in aut_0 and aut_2
    REQUIRE(!is_included_lazy(aut_1, aut_0, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(!is_included_lazy(aut_1, aut_0, nullptr, logging::VerbosityLevel::DEBUGV, false));
    REQUIRE(!is_included_lazy(aut_1, aut_2, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(!is_included_lazy(aut_1, aut_2, nullptr, logging::VerbosityLevel::DEBUGV, false));
    REQUIRE(!is_included_antichains(aut_1, aut_0, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(!is_included_antichains(aut_1, aut_0, nullptr, logging::VerbosityLevel::DEBUGV, false));
    REQUIRE(!is_included_antichains(aut_1, aut_2, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(!is_included_antichains(aut_1, aut_2, nullptr, logging::VerbosityLevel::DEBUGV, false));

    // aut_2 is not contained in aut_0 and aut_1
    REQUIRE(!is_included_lazy(aut_2, aut_0, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(!is_included_lazy(aut_2, aut_0, nullptr, logging::VerbosityLevel::DEBUGV, false));
    REQUIRE(!is_included_lazy(aut_2, aut_1, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(!is_included_lazy(aut_2, aut_1, nullptr, logging::VerbosityLevel::DEBUGV, false));
    REQUIRE(!is_included_antichains(aut_2, aut_0, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(!is_included_antichains(aut_2, aut_0, nullptr, logging::VerbosityLevel::DEBUGV, false));
    REQUIRE(!is_included_antichains(aut_2, aut_1, nullptr, logging::VerbosityLevel::DEBUGV, true));
    REQUIRE(!is_included_antichains(aut_2, aut_1, nullptr, logging::VerbosityLevel::DEBUGV, false));
}

TEST_CASE( "BDDlikeNft universality", "[BDDlikeNft::is_universal_lazy, BDDlikeNft::is_universal_antichains]" ) {
    using namespace mata::ext::bddlike;

    auto print_run = [](mata::nft::Run cex) -> void {
        std::cout << "Run:\n";
        std::cout << '\t' << vec_to_string(cex.word) << std::endl;
        std::cout << "Trace in NFT:\n";
        std::cout << '\t' << vec_to_string(cex.path) << std::endl;
    };

    mata::OnTheFlyAlphabet alph(std::vector<std::string>{"req", "syn", "ack"});
    SimpleVecAlphabet vec_alph_0(std::make_shared<mata::OnTheFlyAlphabet>(alph));
    BoundedDefaultVecAlphabet vec_alph_1(2, 3);

    std::vector<std::shared_ptr<VecAlphabetPrinter>> vec_alph_vec{};
    vec_alph_vec.emplace_back(std::make_shared<SimpleVecAlphabet>(vec_alph_0));
    vec_alph_vec.emplace_back(std::make_shared<BoundedDefaultVecAlphabet>(vec_alph_1));

    SECTION("complement of empty nft") {
        BDDlikeNft aut_emptyset = BDDlikeNft::with_alphabet_sizes({1, 3}, 1, {0}, {}, std::make_optional<std::vector<std::shared_ptr<VecAlphabetPrinter>>>(vec_alph_vec));

        BDDlikeNft aut_comp = complement(aut_emptyset, false);
        BDDlikeNft aut_comp_min = complement(aut_emptyset, true);

        std::cout << "empty nft:\n";
        std::cout << aut_emptyset.print_to_dot_using_alphabets();
        std::cout << "complement:\n";
        std::cout << aut_comp.print_to_dot_using_alphabets();
        std::cout << "complement (min):\n";
        std::cout << aut_comp_min.print_to_dot_using_alphabets();

        REQUIRE(is_universal_lazy(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(is_universal_lazy(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(is_universal_antichains(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(is_universal_antichains(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(is_universal_lazy(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(is_universal_lazy(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(is_universal_antichains(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(is_universal_antichains(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, false));
    }

    SECTION("L(epsilon) and complement of it") {
        BDDlikeNft aut_emptyword = BDDlikeNft::with_alphabet_sizes({1, 3}, 1, {0}, {0}, std::make_optional<std::vector<std::shared_ptr<VecAlphabetPrinter>>>(vec_alph_vec));

        BDDlikeNft aut_comp = complement(aut_emptyword, false);
        BDDlikeNft aut_comp_min = complement(aut_emptyword, true);

        REQUIRE(!is_universal_lazy(aut_emptyword, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_lazy(aut_emptyword, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(!is_universal_antichains(aut_emptyword, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_antichains(aut_emptyword, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(!is_universal_lazy(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_lazy(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(!is_universal_antichains(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_antichains(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(!is_universal_lazy(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_lazy(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(!is_universal_antichains(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_antichains(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, false));
    }

    SECTION("L(w) and complement of it") {
        BDDlikeNft aut = BDDlikeNft::with_alphabet_sizes({1, 3}, 2, {0}, {1}, std::make_optional<std::vector<std::shared_ptr<VecAlphabetPrinter>>>(vec_alph_vec));

        insert_word(aut, 0, std::vector<std::string>{"syn", "ack", "req"}, std::vector<std::vector<mata::Symbol>>{{0, 0, 0}, {0, 1, 1}, {0, 1, 1}}, 1);

        BDDlikeNft aut_comp = complement(aut, false);
        BDDlikeNft aut_comp_min = complement(aut, true);

        REQUIRE(!is_universal_lazy(aut, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_lazy(aut, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(!is_universal_antichains(aut, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_antichains(aut, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(!is_universal_lazy(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_lazy(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(!is_universal_antichains(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_antichains(aut_comp, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(!is_universal_lazy(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_lazy(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, false));
        REQUIRE(!is_universal_antichains(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, true));
        REQUIRE(!is_universal_antichains(aut_comp_min, nullptr, logging::VerbosityLevel::DEBUGV, false));
    }
}
