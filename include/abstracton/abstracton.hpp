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
mata::nfa::Nfa compute_ind(const mata::nft::Nft& abstraction_framework, const mata::nft::Nft& transition_relation, mata::Alphabet& concrete_alphabet, mata::Alphabet& abstract_alphabet, bool exclude_empty_abstractions = false, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL, bool measure_time = false, bool no_dot_printing = false);

// input: DETERMINISTIC abstraction framework!
mata::nft::Nft compute_preach_complement(const mata::nft::Nft& abstraction_framework, const mata::nft::Nft& transition_relation, mata::Alphabet& concrete_alphabet, mata::Alphabet& abstract_alphabet, std::optional<const mata::nfa::Nfa> ind, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL, bool measure_time = false, bool no_dot_printing = false);
mata::nft::Nft compute_preach(const mata::nft::Nft& abstraction_framework, const mata::nft::Nft& transition_relation, mata::Alphabet& concrete_alphabet, mata::Alphabet& abstract_alphabet, std::optional<const mata::nfa::Nfa> ind, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL);

// returns a vector is_safe, where is_safe[i] iff property unsafe_properties[i] could be sepearated from initial configurations in preach
// unsafe properties are given by value, I could not make std::vector<std::reference_wrapper<mata::nfa::Nfa>> to work
// TODO refactor: always take preach_complement as input, but complement in explicit approach
std::vector<bool> check_abstract_safety_explicit(const mata::nfa::Nfa& initial_configurations, const mata::nft::Nft& preach, std::vector<mata::nfa::Nfa> unsafe_properties, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL, bool measure_time = false, bool no_dot_printing = false);
std::vector<bool> check_abstract_safety_lazy(const mata::nfa::Nfa& initial_configurations, const mata::nft::Nft& preach_complement, std::vector<mata::nfa::Nfa> unsafe_properties, mata::Alphabet& concrete_alphabet, std::string universality_alg = "antichains", int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL, bool measure_time = false, bool no_dot_printing = false);
