#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>
#include <abstracton/abstracton.hpp>
#include <abstracton/utils/utils.hpp>
#include <abstracton/mata_extensions.hpp>

using namespace mata;
using namespace mata::nfa;
using namespace mata::nft;

int main() {

    for (auto t : BoundedTuples(std::vector<unsigned int> {1, 2, 3})) {
        for (auto i : t) {
            std::cout << i;
        }
        std::cout << std::endl;
    }

    // transition relation: swap 1s / 0s
    Nft transition_relation {Nft::with_levels(2)};
    State initial {transition_relation.add_state()};
    transition_relation.initial.insert(initial);
    transition_relation.add_transition(initial, {'0', '1'}, initial);
    transition_relation.add_transition(initial, {'1', '0'}, initial);
    transition_relation.final.insert(initial);
    
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
    State af_2x {af.add_transition(af_2, {'#', '1'})};
    af.add_transition(af_2x, {'#', '1'}, af_e);
    af.final.insert(af_e);
    std::cout << af.print_to_dot(true) << std::endl;
    Nft daf {mata::ext::determinize(af)}; // TODO report bug: levels get stripped
    // work around: move to nfa, then construct nft from it with advancing levels


    std::cout << "computing ind" << std::endl;
    Nfa ind {compute_ind(daf, transition_relation, EnumAlphabet {'0', '1', 'a', 'b', '#'})};
    std::cout << "done" << std::endl;
    std::cout << ind.print_to_dot(true) << std::endl;

    // Nft tc {mata::nft::complement(transition_relation, EnumAlphabet {'0', '1'})};
    // Nft id {create_identity(EnumAlphabet {'0', '1'})};
    // Nfa aut {};
    // State s0 {aut.add_state()};
    // State s1 {aut.add_state()};
    // State s2 {aut.add_state()};
    // aut.initial.insert(s0);
    // aut.final.insert(s2);
    // aut.delta.add(s0, '0', s1);
    // aut.delta.add(s1, '1', s1);
    // aut.delta.add(s1, '0', s2);
    // Nft ida {mata::nft::builder::from_nfa_with_levels_zero(aut)};


    return 0;
}
