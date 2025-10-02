#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>
#include <abstracton/abstracton.hpp>

using namespace mata;
using namespace mata::nfa;
using namespace mata::nft;

int main() {
    // transition relation: swap 1s / 0s
    Nft transition_relation {Nft::with_levels(2)};
    State initial { transition_relation.add_state() };
    transition_relation.initial.insert(initial);
    State nextState {transition_relation.add_transition(initial, {'0', '1'})};
    transition_relation.final.insert(nextState);
    // length-preserving abstraction framework:
    // - a## encodes {000, 111} (i.e. all letters are the same)
    // - b## encodes {000} (i.e. all letters are 0)

    Nft tc {mata::nft::complement(transition_relation, EnumAlphabet {'0', '1'})};
    Nft id {create_identity(EnumAlphabet {'0', '1'})};
    Nfa aut {};
    State s0 {aut.add_state()};
    State s1 {aut.add_state()};
    State s2 {aut.add_state()};
    aut.initial.insert(s0);
    aut.final.insert(s2);
    aut.delta.add(s0, '0', s1);
    aut.delta.add(s1, '1', s1);
    aut.delta.add(s1, '0', s2);
    Nft ida {mata::nft::builder::from_nfa_with_levels_zero(aut)};
    std::cout << ida.print_to_dot(true) << std::endl;

    return 0;
}
