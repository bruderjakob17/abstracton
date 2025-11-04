#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>

mata::nft::Nft create_identity(const mata::Alphabet& alphabet);

mata::nft::Nft create_identity(const mata::nfa::Nfa& language);

mata::nfa::Nfa project(const mata::nft::Nft& nft, int level);

namespace mata::ext {

// requires nft to not have epsilon-transitions and be trimmed
mata::nft::Nft determinize(const mata::nft::Nft& nft);

// uses hopcroft minimization (requires trimmed deterministic automaton without epsilon transitions)
mata::nft::Nft minimize(const mata::nft::Nft& nft);

// uses hopcroft minimzation (requires nft to not have epsilon-transitions and be trimmed)
mata::nft::Nft det_and_min(const mata::nft::Nft& nft);

// computes complement of nft
// contrary to mata's complement, this only complements final states on level 0'
mata::nft::Nft complement(const mata::nft::Nft& nft, const mata::Alphabet& alphabet, bool minimize_during_determinization = false);

// completes the given nfa in-place wrt. the padding closure as in Algorithm 24 of Esparza et Blondin's "Automata Theory: An Algorithmic Approach"
void padding_closure(mata::nfa::Nfa& nfa, Symbol padding_symbol);

// completes the given nft in-place wrt. the transition function: adds missing transitions to each state at each level
// does not reuse any sink state, contrary to mata's make_complete. Instead, it just adds an aray of states leading
// to a sink state, where each level is contained in the array.
void make_complete(mata::nft::Nft& nft, const mata::utils::OrdVector<Symbol>& symbols);

namespace builder {

/**
* Creates Tabakov-Vardi random NFT.
* The implementation is based on the paper "Experimental Evaluation of Classical Automata Constructions" by Tabakov and Vardi.
*
* This function essentially works as follows:
* - We interpret NFTs as being NFAs on the alphabet Sigma_1 x ... x Sigma_k, where Sigma_i is the alphabet of level i
* - A random NFA is constructed over this product alphabet
* - The NFT it corresponds to is returned
*
* @param num_of_levels Number of levels of the transducer
* @param num_of_states Number of states in the transducer (on level 0)
* @param alphabet_sizes Sizes of the alphabets on each level. Length of the vector must be num_of_levels.
* @param states_transitions_ratio_per_symbol Ratio between number of transitions and number of states for each symbol.
*  The value must be in range [0, num_of_states]. A value of 1 means that there will be num_of_states transitions for each symbol.
*  A value of num_of_states means that there will be a transition between every pair of states for each symbol.
* @param final_state_density Density of final states in the automaton. The value must be in range [0, 1]. The state 0 is always final.
*  If the density is 1, every state will be final.
*/
mata::nft::Nft create_random_nft_tabakov_vardi(const size_t num_of_levels, const size_t num_of_states, const std::vector<size_t>& alphabet_sizes, const double states_trans_ratio_per_symbol, const double final_state_density);

}

}
