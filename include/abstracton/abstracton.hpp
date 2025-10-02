#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>

using namespace mata;
using namespace mata::nfa;
using namespace mata::nft;

mata::nft::Nft create_identity(const mata::Alphabet& alphabet);

mata::nft::Nft create_identity(const mata::nfa::Nfa& language);

mata::nfa::Nfa project(const mata::nft::Nft& nft, int level);

// input: DETERMINISTIC abstraction framework!
mata::nfa::Nfa compute_ind(const mata::nft::Nft& abstraction_framework, const mata::nft::Nft& transition_relation, const mata::Alphabet& alphabet);
mata::nft::Nft compute_preach(const mata::nft::Nft& abstraction_framework, const mata::nft::Nft& transition_relation, const mata::Alphabet& alphabet);


