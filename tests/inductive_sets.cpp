#include <catch2/catch_test_macros.hpp>
#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <abstracton/mata_extensions.hpp>
#include <abstracton/abstracton.hpp>

TEST_CASE( "Inductive sets are correct", "[compute_ind]" ) {
    using namespace mata;
    using namespace mata::nfa;
    using namespace mata::nft;

    // length-preserving abstraction framework:
    // - a## encodes {000, 111} (i.e. all letters are the same)
    // - b## encodes {000} (i.e. all letters are 0)
    // so, transducer needs to accept (a##, 000), (a##, 111), (b##, 000)
    Nft af {Nft::with_levels(2)};
    State af_init {af.add_state()};
    af.initial.insert(af_init);
    State af_1 {af.add_transition(af_init, {'a', '0'})};
    af.add_transition(af_init, {'b', '0'}, af_1);
    State af_2 {af.add_transition(af_init, {'a', '1'})};
    State af_1x {af.add_transition(af_1, {'#', '0'})};
    State af_e {af.add_transition(af_1x, {'#', '0'})};
    State af_2x {af.add_transition(af_2, {'#', '1'})};
    af.add_transition(af_2x, {'#', '1'}, af_e);
    af.final.insert(af_e);

    Nft daf {mata::ext::determinize(af)};
    std::cout << daf.print_to_dot(true) << std::endl;

    // transducer: flip 0s and 1s
    Nft t {Nft::with_levels(2)};
    State t_init {t.add_state()};
    t.initial.insert(t_init);
    t.final.insert(t_init);
    t.add_transition(t_init, {'0', '1'}, t_init);
    t.add_transition(t_init, {'1', '0'}, t_init);

    std::cout << t.print_to_dot(true) << std::endl;

    mata::EnumAlphabet concrete_alphabet = {'0', '1'};
    mata::EnumAlphabet abstract_alphabet = {'#', 'a', 'b'};

    Nfa ind {compute_ind(daf, t, concrete_alphabet, abstract_alphabet, true, logging::VerbosityLevel::DEBUG)};

    std::cout << ind.print_to_dot(true) << std::endl;

    REQUIRE(ind.get_words(4) == std::set<Word> {
        {'a', '#', '#'},
    });
}
