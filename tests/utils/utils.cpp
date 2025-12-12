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


TEST_CASE( "Subset Iterator correct", "[Subsets(std::unordered_set<T>)]" ) {
    std::vector<std::unordered_set<int>> test_sets {};
    for (int i = 0; i < 3; ++i) {
        test_sets.push_back(std::unordered_set<int>{});
    }
    test_sets[1].insert(3);
    test_sets[2].insert(3);
    test_sets[2].insert(7);
    test_sets[2].insert(10);

    std::vector<std::unordered_set<std::unordered_set<int>>> results {};

    for (int i = 0; i < test_sets.size(); ++i) {
        Subsets s(test_sets[i]);
        std::unordered_set<std::unordered_set<int>> power_set(s.begin(), s.end());
        REQUIRE((1 << test_sets[i].size()) == power_set.size());
        results.push_back(power_set);
    }

    REQUIRE(std::find(results[2].begin(), results[2].end(), std::unordered_set<int>{10, 3, 7}) != results[2].end());
    REQUIRE(std::find(results[2].begin(), results[2].end(), std::unordered_set<int>{3, 7, 10}) != results[2].end());
}


TEST_CASE( "Subsequence Iterator correct", "[SubsequenceVectors(std::vector<T>)]" ) {
    std::vector<std::vector<int>> test_vecs {};
    for (int i = 0; i < 3; ++i) {
        test_vecs.push_back(std::vector<int>{});
    }
    test_vecs[1].push_back(3);
    test_vecs[2].push_back(10);
    test_vecs[2].push_back(3);
    test_vecs[2].push_back(7);

    std::vector<std::unordered_set<std::vector<int>>> results {};

    for (int i = 0; i < test_vecs.size(); ++i) {
        SubsequenceVectors s(test_vecs[i]);
        std::unordered_set<std::vector<int>> power_set(s.begin(), s.end());
        REQUIRE((1 << test_vecs[i].size()) == power_set.size());
        results.push_back(power_set);
        std::cout << "power set of " << vec_to_string(test_vecs[i]) << ":\n";
        for (auto x : power_set) {
            std::cout << "\t" << vec_to_string(x) << "\n";
        }
    }

    REQUIRE(std::find(results[2].begin(), results[2].end(), std::vector<int>{10, 3, 7}) != results[2].end());
    REQUIRE(std::find(results[2].begin(), results[2].end(), std::vector<int>{3, 7, 10}) == results[2].end());
}
