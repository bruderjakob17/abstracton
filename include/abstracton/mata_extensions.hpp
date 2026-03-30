#pragma once

#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nfa/algorithms.hh>
#include <mata/nft/algorithms.hh>
#include <mata/nfa/types.hh>
#include <mata/nft/types.hh>

mata::nft::Nft create_identity(mata::Alphabet& alphabet);

mata::nft::Nft create_identity(const mata::nfa::Nfa& language);

mata::nfa::Nfa project(const mata::nft::Nft& nft, int level);

// minimizes a given nfa using hopcroft minimization. Wrapper for mata minimization, but don't have to worry about trimming/determinizing
inline mata::nfa::Nfa minimize_nfa(const mata::nfa::Nfa& aut) {
    mata::nfa::Nfa aut_det {mata::nfa::determinize(aut)};
    aut_det = aut_det.trim();
    mata::nfa::Nfa aut_min {mata::nfa::algorithms::minimize_hopcroft(aut_det)};
    return aut_min;
}

// applies the given nft to the given nfa on the specified level of the nft (0 or 1).
// level = 0: apply = postimage, level = 1: apply = preimage
// nft must have exactly two levels.
mata::nfa::Nfa apply(const mata::nft::Nft& nft, const mata::nfa::Nfa& nfa, int level = 0);

namespace mata::ext {

// requires nft to not have epsilon-transitions and be trimmed
mata::nft::Nft determinize(const mata::nft::Nft& nft);

// uses hopcroft minimization (requires nft to not have epsilon-transitions and be trimmed)
mata::nft::Nft minimize(const mata::nft::Nft& nft);

// computes complement of nft
// contrary to mata's complement, this only complements final states on level 0'
mata::nft::Nft complement(const mata::nft::Nft& nft, const std::vector<mata::utils::OrdVector<Symbol>>& symbols, bool minimize_during_determinization = false);
mata::nft::Nft complement(const mata::nft::Nft& nft, const Alphabet* alphabet = nullptr, const std::optional<const std::vector<Alphabet*>> alphabets = std::nullopt, bool minimize_during_determinization = false);

// some constructions for same-length nfts
/**
 * Constructs a *length-preserving* NFT accepting all words over the given alphabet(s). For examples, see the tests.
 */
mata::nft::Nft create_sigma_star_nft(int number_of_levels, Alphabet* alphabet = nullptr, const std::optional<const std::vector<Alphabet*>> alphabets = std::nullopt);

/**
 * Universality checking based on subset construction with antichain.
 * @param[in] aut Automaton which universality is checked
 * @param[in] alphabets Alphabets of the automaton
 * @param[out] cex Counterexample word which eventually breaks the universality
 * @return True if the automaton is universal, otherwise false. IMPORTANT: in contrast to mata's mata::nft::algorithms::is_universal_antichains, this algorithm checks only universality w.r.t. words which have the same length on each tape
 */
bool is_universal_antichains(const mata::nft::Nft& aut, const std::vector<Alphabet*> alphabets, mata::nft::Run* cex);

/// similar to is_universal_antichains, but solves the problem by just calling inclusion algorithm to check if sigma star (length-preserving version) is included
/// Note: this check seems to be a LOT slower than is_universal_antichains, see examples/compare_inclusion_checks.cpp. The reason might be that @param alphabets is only used to create sigma star, and for the inclusion, mata's is_included_antichains is called, which does not support tape-specific alphabets, meaning it constructs an alphabet valid for all tapes by iterating over the whole automaton
bool is_universal_antichains_by_inclusion(const mata::nft::Nft& aut, const std::vector<Alphabet*> alphabets, mata::nft::Run* cex);

/// similar to is_universal_antichains, but do not do subsumption checks
bool is_universal_lazy(const mata::nft::Nft& aut, const std::vector<Alphabet*> alphabets, mata::nft::Run* cex);

/**
 * Inserts tapes to construct a new NFT. Assumes @param aut is length-preserving.
 * It is equivalent to the "relational length-preserving product" of @param aut with sigma stars specified by @param inserted_tape_alphabets, at positions @param inserted_tape_indices.
 * E.g. if aut accepts {(a, b), (aa, bb)}, inserted_tape_indices is {1, 3} and new_tape_alphabets {{c, d}, {e}}, the resulting nft will accept
 * {(a, c, b, e), (a, d, b, e), (aa, cc, bb, ee), (aa, cd, bb, ee), (aa, dc, bb, ee), (aa, dd, bb, ee)}
 *
 * Restrictions:
 * - @param inserted_tape_indices must be strictly increasing
 * - all states in @param aut must be reachable
 *
 * TODO compare with mata::nft::insert_tapes (which inserts DONT_CARE symbols); i.e. construct that and replace DONT_CAREs with corresp. alphabets
 */
mata::nft::Nft insert_tapes(const mata::nft::Nft& aut, const std::vector<int> inserted_tape_indices, const std::vector<Alphabet*> inserted_tape_alphabets);

/**
 * Constructs an NFT accepting the relational product of @param nfts, restricted to same-length-tuples.
 *
 * E.g.: {(ab, aa), (ab, bb)} x {(c, d), (cc, dd)} = {(ab, aa, cc, dd), (ab, bb, cc, dd)}
 */
mata::nft::Nft relational_product_length_preserving(const std::vector<mata::nft::Nft> nfts);

/**
 * same as relational_product_length_preserving, but does not need to exctract any alphabets
 */
mata::nft::Nft relational_product_length_preserving_dont_care(const std::vector<mata::nft::Nft> nfts);

// completes the given nfa in-place wrt. the padding closure as in Algorithm 24 of Esparza et Blondin's "Automata Theory: An Algorithmic Approach"
void padding_closure(mata::nfa::Nfa& nfa, Symbol padding_symbol);

// completes the given nft in-place wrt. the transition function: adds missing transitions to each state at each level
// does not reuse any sink state, contrary to mata's make_complete. Instead, it just adds an aray of states leading
// to a sink state, where each level is contained in the array.
void make_complete(mata::nft::Nft& nft, const std::vector<mata::utils::OrdVector<Symbol>>& symbols);
void make_complete(mata::nft::Nft& nft, const Alphabet* alphabet = nullptr, const std::optional<const std::vector<Alphabet*>> alphabets = std::nullopt);

std::vector<mata::utils::OrdVector<Symbol>> get_tape_symbols_to_work_with(const mata::nft::Nft& nft, const Alphabet* alphabet = nullptr, const std::optional<const std::vector<Alphabet*>> alphabets = std::nullopt);

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
mata::nft::Nft create_random_nft_tabakov_vardi(const size_t num_of_levels, const size_t num_of_states, const std::vector<size_t>& alphabet_sizes, const double states_trans_ratio_per_symbol, const double final_state_density, const std::optional<unsigned int> seed = std::nullopt);

}

}
