#include <catch2/catch_test_macros.hpp>
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
