#include <catch2/catch_test_macros.hpp>
#include <abstracton/interpretations.hpp>
#include <abstracton/utils/utils.hpp>

TEST_CASE("Trap interpretation") {
    using namespace mata;

    mata::OnTheFlyAlphabet string_alphabet{std::vector<std::string>{std::string("a"), std::string("b")}};

    std::pair<mata::nft::Nft, std::shared_ptr<mata::OnTheFlyAlphabet>> ipa = trapInterpretation(&string_alphabet, SetInterpretation::Trap);
    mata::nft::Nft interpretation = ipa.first;
    std::shared_ptr<mata::OnTheFlyAlphabet> powerset_alphabet_ptr = ipa.second;
    std::cout << "trap interpretation:\n";
    std::cout << interpretation.print_to_dot() << std::endl;

    // abaaa in V({b}{b}{b}{b}{b})
    WordName word1name {"b", "a", "a", "a", "a"};
    WordName word2name {"[b]", "[b]", "[b]", "[b]", "[]"};
    Word word1 = string_alphabet.translate_word(word1name);
    Word word2 = powerset_alphabet_ptr->translate_word(word2name);
    std::cout << "word1: " << vec_to_string(word1) << "\n";
    std::cout << "word2: " << vec_to_string(word2) << "\n";
    REQUIRE(interpretation.is_in_lang_by_levels({word2, word1}));

    // ababa not in V({b}{b}{b}{b}{b})
    WordName abaaaName {"a", "b", "a", "a", "a"};
    Word abaaa = string_alphabet.translate_word(abaaaName);
    std::cout << "abaaa: " << vec_to_string(abaaa) << "\n";
    REQUIRE(interpretation.is_in_lang_by_levels({word2, abaaa}));

    // bbbba not in V({b}{b}{b}{b}{b})
    WordName aaaaaName {"a", "a", "a", "a", "a"};
    Word aaaaa = string_alphabet.translate_word(aaaaaName);
    std::cout << "aaaaa: " << vec_to_string(aaaaa) << "\n";
    REQUIRE(!interpretation.is_in_lang_by_levels({word2, aaaaa}));
}

TEST_CASE("Siphon interpretation") {
    using namespace mata;

    mata::OnTheFlyAlphabet string_alphabet{std::vector<std::string>{std::string("a"), std::string("b")}};

    std::pair<mata::nft::Nft, std::shared_ptr<mata::OnTheFlyAlphabet>> ipa = trapInterpretation(&string_alphabet, SetInterpretation::Siphon);
    mata::nft::Nft interpretation = ipa.first;
    std::shared_ptr<mata::OnTheFlyAlphabet> powerset_alphabet_ptr = ipa.second;
    std::cout << "siphon interpretation:\n";
    std::cout << interpretation.print_to_dot() << std::endl;

    // abaaa in V({b}{b}{b}{b}{b})
    WordName word1name {"a", "a", "a", "a", "a"};
    WordName word2name {"[b]", "[b]", "[b]", "[b]", "[]"};
    Word word1 = string_alphabet.translate_word(word1name);
    Word word2 = powerset_alphabet_ptr->translate_word(word2name);
    std::cout << "word1: " << vec_to_string(word1) << "\n";
    std::cout << "word2: " << vec_to_string(word2) << "\n";
    REQUIRE(interpretation.is_in_lang_by_levels({word2, word1}));

    // ababa not in V({b}{b}{b}{b}{b})
    WordName abaaaName {"a", "b", "a", "a", "a"};
    Word abaaa = string_alphabet.translate_word(abaaaName);
    std::cout << "abaaa: " << vec_to_string(abaaa) << "\n";
    REQUIRE(!interpretation.is_in_lang_by_levels({word2, abaaa}));

    // bbbba not in V({b}{b}{b}{b}{b})
    WordName bbbbaName {"b", "b", "b", "b", "a"};
    Word bbbba = string_alphabet.translate_word(bbbbaName);
    std::cout << "bbbba: " << vec_to_string(bbbba) << "\n";
    REQUIRE(!interpretation.is_in_lang_by_levels({word2, bbbba}));
}

TEST_CASE("Flow interpretation") {
    using namespace mata;

    OnTheFlyAlphabet string_alphabet{std::vector<std::string>{std::string("a"), std::string("b")}};

    std::pair<mata::nft::Nft, std::shared_ptr<OnTheFlyAlphabet>> ipa = trapInterpretation(&string_alphabet, SetInterpretation::Flow);
    mata::nft::Nft interpretation = ipa.first;
    std::shared_ptr<OnTheFlyAlphabet> powerset_alphabet_ptr = ipa.second;
    std::cout << "flow interpretation:\n";
    std::cout << interpretation.print_to_dot() << std::endl;

    // abaaa in V({b}{b}{b}{b}{b})
    WordName word1name {"a", "b", "a", "a", "a"};
    WordName word2name {"[b]", "[b]", "[b]", "[b]", "[b]"};
    Word word1 = string_alphabet.translate_word(word1name);
    Word word2 = powerset_alphabet_ptr->translate_word(word2name);
    std::cout << "word1: " << vec_to_string(word1) << "\n";
    std::cout << "word2: " << vec_to_string(word2) << "\n";
    REQUIRE(interpretation.is_in_lang_by_levels({word2, word1}));

    // ababa not in V({b}{b}{b}{b}{b})
    WordName ababaName {"a", "b", "a", "b", "a"};
    Word ababa = string_alphabet.translate_word(ababaName);
    std::cout << "ababa: " << vec_to_string(ababa) << "\n";
    REQUIRE(!interpretation.is_in_lang_by_levels({word2, ababa}));

    // bbbba not in V({b}{b}{b}{b}{b})
    WordName bbbbaName {"b", "b", "b", "b", "a"};
    Word bbbba = string_alphabet.translate_word(bbbbaName);
    std::cout << "bbbba: " << vec_to_string(bbbba) << "\n";
    REQUIRE(!interpretation.is_in_lang_by_levels({word2, bbbba}));

    // mata::nft::State current_state{ 0 };
    // std::cout << "traversal:\n-> 0\n";
    // for (int i = 0; i < 5; ++i) {
    //     std::cout << "-" << word2name[i] << "-> ";
    //     current_state = interpretation.delta.get_successors(current_state, word2[i]).front();
    //     std::cout << current_state << ":" << interpretation.levels[current_state] << std::endl;
    //     std::cout << "-" << bbbbaName[i] << "-> ";
    //     current_state = interpretation.delta.get_successors(current_state, bbbba[i]).front();
    //     std::cout << current_state << ":" << interpretation.levels[current_state] << std::endl;
    // }
}
