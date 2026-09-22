#include <abstracton/bddlike/interpretations.hpp>

#include <catch2/catch_test_macros.hpp>
#include <abstracton/interpretations.hpp>
#include <abstracton/utils/utils.hpp>

TEST_CASE("Trap interpretation (BDDlike)", "[interpretation]") {
    using namespace mata;
    using namespace mata::ext::bddlike;
    using namespace std;

    vector<vector<string>> abstract_word = {{"b"}, {"b"}, {}};
    vector<string> aaa = {"a", "a", "a"};
    vector<string> aba = {"a", "b", "a"};
    vector<string> bba = {"b", "b", "a"};
    vector<string> too_short = {"b", "b"};
    vector<string> too_long = {"b", "b", "b", "b"};

    OnTheFlyAlphabet mata_string_alphabet{vector<string>{string("a"), string("b")}};

    SECTION("Trap") {
        BDDlikeNft interpretation = mata::ext::bddlike::trapInterpretation(make_shared<OnTheFlyAlphabet>(mata_string_alphabet), SetInterpretation::Trap);

        shared_ptr<SimpleVecAlphabet> string_alphabet = static_pointer_cast<SimpleVecAlphabet>(interpretation.alphabets[0]);
        shared_ptr<PowersetVecAlphabet> powerset_alphabet = static_pointer_cast<PowersetVecAlphabet>(interpretation.alphabets[1]);

        cout << "Trap interpretation:\n";
        cout << interpretation.print_to_dot_using_alphabets() << endl;

        CHECK_FALSE(interpretation.is_in_lang_by_levels({
            string_alphabet->translate_word(aaa),
            powerset_alphabet->translate_word(abstract_word)
        }));
        CHECK(interpretation.is_in_lang_by_levels({
            string_alphabet->translate_word(aba),
            powerset_alphabet->translate_word(abstract_word)
        }));
        CHECK(interpretation.is_in_lang_by_levels({
            string_alphabet->translate_word(bba),
            powerset_alphabet->translate_word(abstract_word)
        }));
        CHECK_FALSE(interpretation.is_in_lang_by_levels({string_alphabet->translate_word(too_short), powerset_alphabet->translate_word(abstract_word)}));
        CHECK_FALSE(interpretation.is_in_lang_by_levels({string_alphabet->translate_word(too_long), powerset_alphabet->translate_word(abstract_word)}));

        CHECK(interpretation.is_deterministic());
    }

    SECTION("Big Trap") {
        vector<string> letters{};
        for (int i{ 0 }; i < 50; ++i) {
            letters.push_back(to_string(i));
        }
        OnTheFlyAlphabet msa{letters};
        BDDlikeNft interpretation = mata::ext::bddlike::trapInterpretation(make_shared<OnTheFlyAlphabet>(msa), SetInterpretation::Trap);

        std::cout << "internal representation:\n";
        std::cout << interpretation.num_of_states() << std::endl;

        // only requirement: test does not overflow memory
        REQUIRE(true);
    }

    SECTION("Siphon") {
        BDDlikeNft interpretation = mata::ext::bddlike::trapInterpretation(make_shared<OnTheFlyAlphabet>(mata_string_alphabet), SetInterpretation::Siphon);

        shared_ptr<SimpleVecAlphabet> string_alphabet = static_pointer_cast<SimpleVecAlphabet>(interpretation.alphabets[0]);
        shared_ptr<PowersetVecAlphabet> powerset_alphabet = static_pointer_cast<PowersetVecAlphabet>(interpretation.alphabets[1]);

        cout << "Siphon interpretation:\n";
        cout << interpretation.print_to_dot_using_alphabets() << endl;

        CHECK(interpretation.is_in_lang_by_levels({
            string_alphabet->translate_word(aaa),
            powerset_alphabet->translate_word(abstract_word)
        }));
        CHECK_FALSE(interpretation.is_in_lang_by_levels({
            string_alphabet->translate_word(aba),
            powerset_alphabet->translate_word(abstract_word)
        }));
        CHECK_FALSE(interpretation.is_in_lang_by_levels({
            string_alphabet->translate_word(bba),
            powerset_alphabet->translate_word(abstract_word)
        }));
        CHECK_FALSE(interpretation.is_in_lang_by_levels({string_alphabet->translate_word(too_short), powerset_alphabet->translate_word(abstract_word)}));
        CHECK_FALSE(interpretation.is_in_lang_by_levels({string_alphabet->translate_word(too_long), powerset_alphabet->translate_word(abstract_word)}));

        CHECK(interpretation.is_deterministic());
    }

    SECTION("Flow") {
        BDDlikeNft interpretation = mata::ext::bddlike::trapInterpretation(make_shared<OnTheFlyAlphabet>(mata_string_alphabet), SetInterpretation::Flow);

        shared_ptr<SimpleVecAlphabet> string_alphabet = static_pointer_cast<SimpleVecAlphabet>(interpretation.alphabets[0]);
        shared_ptr<PowersetVecAlphabet> powerset_alphabet = static_pointer_cast<PowersetVecAlphabet>(interpretation.alphabets[1]);

        cout << "Flow interpretation:\n";
        cout << interpretation.print_to_dot_using_alphabets() << endl;

        CHECK_FALSE(interpretation.is_in_lang_by_levels({
            string_alphabet->translate_word(aaa),
            powerset_alphabet->translate_word(abstract_word)
        }));
        CHECK(interpretation.is_in_lang_by_levels({
            string_alphabet->translate_word(aba),
            powerset_alphabet->translate_word(abstract_word)
        }));
        CHECK_FALSE(interpretation.is_in_lang_by_levels({
            string_alphabet->translate_word(bba),
            powerset_alphabet->translate_word(abstract_word)
        }));
        CHECK_FALSE(interpretation.is_in_lang_by_levels({string_alphabet->translate_word(too_short), powerset_alphabet->translate_word(abstract_word)}));
        CHECK_FALSE(interpretation.is_in_lang_by_levels({string_alphabet->translate_word(too_long), powerset_alphabet->translate_word(abstract_word)}));

        CHECK(interpretation.is_deterministic());
    }
}
