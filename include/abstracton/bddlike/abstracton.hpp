#pragma once

#include <mata/alphabet.hh>
#include <abstracton/utils/utils.hpp>
#include <abstracton/bddlike/bddlike_nft.hpp>

/**
 * input: DETERMINISTIC abstraction framework!
 * the order of tapes in the abstraction framework must be (abstract, concrete).
 * TODO maybe switch to (concrete, abstract) ? this might perform better for set interpretations in bddlike nfts
 */
mata::ext::bddlike::BDDlikeNft compute_ind_new(mata::ext::bddlike::BDDlikeNft& abstraction_framework, mata::ext::bddlike::BDDlikeNft& transition_relation, bool exclude_empty_abstractions = false, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL, bool measure_time = false, bool no_dot_printing = false);


// Input: DETERMINISTIC abstraction framework!
mata::ext::bddlike::BDDlikeNft compute_preach_complement(mata::ext::bddlike::BDDlikeNft& abstraction_framework, mata::ext::bddlike::BDDlikeNft& transition_relation, std::optional<mata::ext::bddlike::BDDlikeNft>& ind, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL, bool measure_time = false, bool no_dot_printing = false);
