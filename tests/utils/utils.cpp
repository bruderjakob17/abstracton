#include <catch2/catch_test_macros.hpp>
#include <mata/parser/re2parser.hh>
#include <abstracton/utils/utils.hpp>

TEST_CASE("vec_to_string correct", "[vec_to_string]") {
    using namespace std;

    // empty vec
    CHECK(vec_to_string(vector<int>{}) == "[]");
    CHECK(vec_to_string(vector<int>{}, "complicated delimiter") == "[]");
    CHECK(vec_to_string(vector<string>{}, "complicated delimiter") == "[]");

    // vec with one element
    CHECK(vec_to_string(vector<int>{17}) == "[17]");
    CHECK(vec_to_string(vector<int>{17}, "complicated delimiter") == "[17]");
    CHECK(vec_to_string(vector<string>{"hello"}, "complicated delimiter") == "[hello]");

    // vec with more elements
    CHECK(vec_to_string(vector<int>{17, 23}) == "[17, 23]");
    CHECK(vec_to_string(vector<int>{17, 23, 29}, ";") == "[17;23;29]");
    CHECK(vec_to_string(vector<string>{"hello", "world", "how", "are", "you"}, "  ") == "[hello  world  how  are  you]");
}

TEST_CASE("vec_sum correct", "[vec_sum(const std::vector<T>&)]") {
    using namespace std;

    vector<int> v_int{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK(vec_sum(v_int) == 55);

    vector<long> v_long{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK(vec_sum(v_long) == 55);

    vector<size_t> v_size_t{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK(vec_sum(v_size_t) == 55);
}

TEST_CASE("split correct", "[split]") {
    using namespace std;

    // "normal" case
    CHECK(split(vector<string>{"a", "b", "c"}, vector<size_t>{1, 0, 0, 2}) == vector<vector<string>>{{"a"}, {}, {}, {"b", "c"}});
    CHECK(split(vector<string>{"a", "b", "c"}, vector<size_t>{2, 1, 0}) == vector<vector<string>>{{"a", "b"}, {"c"}, {}});

    // only one chunk
    CHECK(split(vector<string>{"a", "b", "c"}, vector<size_t>{3}) == vector<vector<string>>{{"a", "b", "c"}});

    // empty vector, one chunk
    CHECK(split(vector<string>{}, vector<size_t>{0}) == vector<vector<string>>{{}});

    // empty vector, no chunk
    CHECK(split(vector<string>{}, vector<size_t>{}) == vector<vector<string>>{});

    // exceeding size
    CHECK_THROWS(split(vector<string>{}, vector<size_t>{1}));
    CHECK_THROWS(split(vector<string>{"a", "b", "c"}, vector<size_t>{1, 2, 1}));
    CHECK_THROWS(split(vector<string>{"a", "b", "c"}, vector<size_t>{2, 2}));
}

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

TEST_CASE( "Vector complement correct", "[vec_complement(const std::vector<T>&, const std::vector<T>&)]") {
    std::vector<std::string> univ {"a", "b"};
    REQUIRE(vec_complement(std::vector<std::string>{}, univ) == univ);
    REQUIRE(vec_complement(std::vector<std::string>{"a"}, univ) == std::vector<std::string>{"b"});
    REQUIRE(vec_complement(std::vector<std::string>{"b"}, univ) == std::vector<std::string>{"a"});
    REQUIRE(vec_complement(univ, univ) == std::vector<std::string>{});
}
