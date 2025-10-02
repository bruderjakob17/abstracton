#include <catch2/catch_test_macros.hpp>
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
