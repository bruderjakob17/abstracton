#include <catch2/catch_test_macros.hpp>
#include <mata/parser/re2parser.hh>
#include <abstracton/mata_extensions.hpp>

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

TEST_CASE("Nft determinization", "[mata::ext::determinize]") {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    // length-preserving abstraction framework:
    // - a## encodes {000, 111} (i.e. all letters are the same)
    // - b## encodes {000} (i.e. all letters are 0)
    // so, transducer needs to accept (a##, 000), (a##, 111), (b##, 000)
    Nft af {Nft::with_levels(2)};
    State af_init {af.add_state() };
    af.initial.insert(af_init);
    State af_1 {af.add_transition(af_init, {'a', '0'})};
    af.add_transition(af_init, {'b', '0'}, af_1);
    State af_2 {af.add_transition(af_init, {'a', '1'})};
    State af_1x {af.add_transition(af_1, {'#', '0'})};
    State af_e {af.add_transition(af_1x, {'#', '0'})};
    State af_2x {af.add_transition(af_2, {'1', '#'})};
    af.add_transition(af_2x, {'1', '#'}, af_e);
    af.final.insert(af_e);

    std::cout << af.print_to_dot(true) << std::endl;

    Nft daf {mata::ext::determinize(af)};

    std::cout << daf.print_to_dot(true) << std::endl;

    REQUIRE(false);
}

TEST_CASE("Padding closure", "[mata::ext::padding_closure]") {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    Nfa aut(4);

    aut.initial = { 0 };
    aut.final = { 3 };
    aut.delta.add(3, 0, 3);
    aut.delta.add(0, 0, 1);
    aut.delta.add(1, 1, 2);
    aut.delta.add(2, 0, 3);

    REQUIRE(!aut.is_in_lang({0, 1}));
std::cout << aut.print_to_dot() << std::endl;
    mata::ext::padding_closure(aut, 0);
std::cout << aut.print_to_dot() << std::endl;
    REQUIRE(aut.is_in_lang({0, 1}));
}
