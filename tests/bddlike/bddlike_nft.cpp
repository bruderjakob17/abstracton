#include <catch2/catch_test_macros.hpp>
#include <mata/nft/nft.hh>
#include <mata/parser/re2parser.hh>
#include <abstracton/bddlike/bddlike_nft.hpp>
#include <abstracton/mata_extensions.hpp>
#include <abstracton/utils/utils.hpp>

TEST_CASE( "bddlike_nft default alphabet", "[bddlike_nft]" ) {
    using namespace mata;

    ext::BDDlikeNft aut = ext::BDDlikeNft::with_alphabet_sizes({2, 3});
    REQUIRE(aut.levels.num_of_levels == 5);
}
