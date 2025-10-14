#include <catch2/catch_test_macros.hpp>
#include <mata/parser/re2parser.hh>
#include <abstracton/abstracton.hpp>

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