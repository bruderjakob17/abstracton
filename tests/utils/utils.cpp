#include <catch2/catch_test_macros.hpp>
#include <mata/parser/re2parser.hh>
#include <abstracton/utils/utils.hpp>

TEST_CASE( "Bounded Tuple Iterator correct", "[BoundedTuples(std::vector<unsigned int>)]" ) {
    BoundedTuples t0(std::vector<unsigned int> {1, 2, 3});
    std::vector<std::vector<unsigned int>> t0_vec(t0.begin(), t0.end());

    REQUIRE(t0_vec == std::vector<std::vector<unsigned int>> {
        {0, 0, 0},
        {0, 0, 1},
        {0, 0, 2},
        {0, 1, 0},
        {0, 1, 1},
        {0, 1, 2}
    });

    BoundedTuples t1(std::vector<unsigned int> {1, 0});
    BoundedTuples t2(std::vector<unsigned int> {0, 0});
    BoundedTuples t3(std::vector<unsigned int> {});
    for (auto i : {t1, t2, t3}) {
        std::vector<std::vector<unsigned int>> t_vec(i.begin(), i.end());
        REQUIRE(t_vec == std::vector<std::vector<unsigned int>> {});
    }
}
