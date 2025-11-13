#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>

// input: DETERMINISTIC abstraction framework!
mata::nfa::Nfa compute_ind(const mata::nft::Nft& abstraction_framework, const mata::nft::Nft& transition_relation, const mata::Alphabet& alphabet, bool exclude_empty_abstractions = false);
mata::nft::Nft compute_preach(const mata::nft::Nft& abstraction_framework, const mata::nft::Nft& transition_relation, const mata::Alphabet& alphabet);


