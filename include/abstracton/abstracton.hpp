#pragma once

#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/alphabet.hh>
#include <abstracton/utils/utils.hpp>

/**
 * input: DETERMINISTIC abstraction framework!
 * the order of tapes in the abstraction framework must be (abstract, concrete).
 * TODO maybe switch to (concrete, abstract) ? this might perform better for set interpretations in bddlike nfts
 */
mata::nfa::Nfa compute_ind(const mata::nft::Nft& abstraction_framework, const mata::nft::Nft& transition_relation, mata::Alphabet& concrete_alphabet, mata::Alphabet& abstract_alphabet, bool exclude_empty_abstractions = false, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL);

// input: DETERMINISTIC abstraction framework!
mata::nft::Nft compute_preach_complement(const mata::nft::Nft& abstraction_framework, const mata::nft::Nft& transition_relation, mata::Alphabet& concrete_alphabet, mata::Alphabet& abstract_alphabet, std::optional<const mata::nfa::Nfa> ind = std::nullopt, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL);
mata::nft::Nft compute_preach(const mata::nft::Nft& abstraction_framework, const mata::nft::Nft& transition_relation, mata::Alphabet& concrete_alphabet, mata::Alphabet& abstract_alphabet, std::optional<const mata::nfa::Nfa> ind = std::nullopt, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL);

// returns a vector is_safe, where is_safe[i] iff property unsafe_properties[i] could be sepearated from initial configurations in preach
// unsafe properties are given by value, I could not make std::vector<std::reference_wrapper<mata::nfa::Nfa>> to work
std::vector<bool> check_abstract_safety(const mata::nfa::Nfa& initial_configurations, const mata::nft::Nft& preach, std::vector<mata::nfa::Nfa> unsafe_properties, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL);
std::vector<bool> check_abstract_safety_antichains(const mata::nfa::Nfa& initial_configurations, const mata::nft::Nft& preach_complement, std::vector<mata::nfa::Nfa> unsafe_properties, mata::Alphabet& concrete_alphabet, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL);
