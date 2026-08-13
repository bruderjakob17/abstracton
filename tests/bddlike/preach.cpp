#include <catch2/catch_test_macros.hpp>
#include <mata/alphabet.hh>
#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <abstracton/mata_extensions.hpp>
#include <abstracton/abstracton.hpp>
#include <abstracton/bddlike/bddlike_nft.hpp>
#include <abstracton/bddlike/abstracton.hpp>

TEST_CASE( "PReach correct (BDDlikeNft)", "[bddlike::compute_preach_complement]" ) {
    using namespace mata;
    using namespace mata::ext::bddlike;

    // length-preserving abstraction framework:
    // - a## encodes {000, 111} (i.e. all letters are the same)
    // - b## encodes {000} (i.e. all letters are 0)
    // so, transducer needs to accept (a##, 000), (a##, 111), (b##, 000)

    // in this bddlike test, we represent "a" by "xx", "b" by "xy", and "#" by "yy".
    // hence, the concrete alphabet is {0, 1}, and the abstract alphabet is {"a", "b", "#"} represented by {"xx", "xy", "yy"}.
    mata::OnTheFlyAlphabet abstract_alph(std::vector<std::string>{"x", "y"});
    AlphabetVecAlphabet abstract_alphabet(std::make_shared<mata::OnTheFlyAlphabet>(abstract_alph), 2);

    mata::EnumAlphabet concrete_alph = {"0", "1"};
    SimpleVecAlphabet concrete_alphabet(std::make_shared<mata::EnumAlphabet>(concrete_alph));

    std::vector<std::shared_ptr<VecAlphabetPrinter>> af_alphabet_vector{};
    af_alphabet_vector.emplace_back(std::make_shared<SimpleVecAlphabet>(concrete_alphabet));
    af_alphabet_vector.emplace_back(std::make_shared<AlphabetVecAlphabet>(abstract_alphabet));

    BDDlikeNft af = BDDlikeNft::with_alphabet_sizes({1, 2}, 2, {0}, {1}, std::make_optional<std::vector<std::shared_ptr<VecAlphabetPrinter>>>(af_alphabet_vector));

    insert_word(af, 0, std::vector<std::string>{"0", "0", "0"}, std::vector<std::vector<std::string>>{{"x", "x"}, {"y", "y"}, {"y", "y"}}, 1);
    insert_word(af, 0, std::vector<std::string>{"1", "1", "1"}, std::vector<std::vector<std::string>>{{"x", "x"}, {"y", "y"}, {"y", "y"}}, 1);
    insert_word(af, 0, std::vector<std::string>{"0", "0", "0"}, std::vector<std::vector<std::string>>{{"x", "y"}, {"y", "y"}, {"y", "y"}}, 1);

    af = mata::ext::bddlike::minimize(af);

    std::cout << "abstraction framework:\n";
    std::cout << af.print_to_dot_using_alphabets() << std::endl;

    // transducer: flip 0s and 1s
    std::vector<std::shared_ptr<VecAlphabetPrinter>> t_alphabet_vector{};
    t_alphabet_vector.emplace_back(std::make_shared<SimpleVecAlphabet>(concrete_alphabet));
    t_alphabet_vector.emplace_back(std::make_shared<SimpleVecAlphabet>(concrete_alphabet));
    BDDlikeNft t = BDDlikeNft::with_alphabet_sizes({1, 1}, 1, {0}, {0}, std::make_optional<std::vector<std::shared_ptr<VecAlphabetPrinter>>>(t_alphabet_vector));

    insert(t, 0, std::string("0"), std::string("1"), 0);
    insert(t, 0, std::string("1"), std::string("0"), 0);

    std::cout << "transition relation:\n";
    std::cout << t.print_to_dot_using_alphabets();

    BDDlikeNft ind = compute_ind_new(af, t, true);

    std::cout << "inductive invariants (calculated):\n";
    std::cout << ind.print_to_dot_using_alphabets();

    REQUIRE(ind.get_words(9) == std::set<Word> {
        flatten(std::vector<std::vector<mata::Symbol>>{abstract_alphabet.translate_symbol({"x", "x"}), abstract_alphabet.translate_symbol({"y", "y"}), abstract_alphabet.translate_symbol({"y", "y"})})
    });

    std::optional<BDDlikeNft> ind_opt = std::make_optional(ind);

    BDDlikeNft preach_comp = compute_preach_complement(af, t, ind_opt, logging::VerbosityLevel::VERBOSE, true, false);

    std::cout << "preach complement (calculated):\n";
    std::cout << preach_comp.print_to_dot_using_alphabets();

    REQUIRE(!preach_comp.is_in_lang_by_levels({
        concrete_alphabet.translate_word({"0", "0", "0"}),
        concrete_alphabet.translate_word({"1", "1", "1"})
    }));
    REQUIRE(!preach_comp.is_in_lang_by_levels({
        concrete_alphabet.translate_word({"1", "1", "1"}),
        concrete_alphabet.translate_word({"0", "0", "0"})
    }));
    REQUIRE(!preach_comp.is_in_lang_by_levels({
        concrete_alphabet.translate_word({"0", "0", "0"}),
        concrete_alphabet.translate_word({"0", "0", "0"})
    }));
    REQUIRE(!preach_comp.is_in_lang_by_levels({
        concrete_alphabet.translate_word({"1", "1", "1"}),
        concrete_alphabet.translate_word({"1", "1", "1"})
    }));
    REQUIRE(preach_comp.is_in_lang_by_levels({
        concrete_alphabet.translate_word({"0", "0", "0"}),
        concrete_alphabet.translate_word({"1", "0", "1"})
    }));
    REQUIRE(preach_comp.is_in_lang_by_levels({
        concrete_alphabet.translate_word({"1", "1", "1"}),
        concrete_alphabet.translate_word({"1", "0", "1"})
    }));
    REQUIRE(!preach_comp.is_in_lang_by_levels({
        concrete_alphabet.translate_word({"0", "1", "1"}),
        concrete_alphabet.translate_word({"0", "0", "0"})
    }));
    REQUIRE(!preach_comp.is_in_lang_by_levels({
        concrete_alphabet.translate_word({"0", "1", "1"}),
        concrete_alphabet.translate_word({"1", "1", "1"})
    }));
    REQUIRE(!preach_comp.is_in_lang_by_levels({
        concrete_alphabet.translate_word({"0", "1", "1"}),
        concrete_alphabet.translate_word({"1", "0", "0"})
    }));
}
