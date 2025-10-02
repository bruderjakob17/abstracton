#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>

using namespace mata;
using namespace mata::nfa;
using namespace mata::nft;

Nft create_identity(const Alphabet& alphabet) {
    Nft result {};
    State initial {result.add_state()};
    result.initial.insert(initial);
    result.final.insert(initial);
    result.insert_identity(initial, &alphabet);
    return result;
}

Nft create_identity(const Nfa& language) {
    return mata::nft::builder::from_nfa_with_levels_zero(language);
}

Nfa project(const Nft& nft, int level) {
    // TODO add possibility to do padding closure wrt. some padding symbol
    Nft nft_proj {mata::nft::project_to(nft, level)};
    return nft_proj.to_nfa_move();
}

// input: DETERMINISTIC abstraction framework!
Nfa compute_ind(const Nft& abstraction_framework, const Nft& transition_relation, const Alphabet& alphabet) {
    // project_1(Id intersect (V delta complement(inverse(V)))), then complement
    Nft v_delta {compose(abstraction_framework, transition_relation)};
    Nft v_complement {mata::nft::complement(abstraction_framework, alphabet)};
    Nft product1 {compose(v_delta, v_complement, 1, 1)};
    Nft preprojection {mata::nft::intersection(create_identity(alphabet), product1)};
    return mata::nfa::complement(project(preprojection, 0), alphabet);
}
Nft compute_preach(const Nft& abstraction_framework, const Nft& transition_relation, const Alphabet& alphabet) {
    // inverse(V) id_Ind complement(V), then complement
    Nfa ind {compute_ind(abstraction_framework, transition_relation, alphabet)};

    Nft v_complement {mata::nft::complement(abstraction_framework, alphabet)}; // TODO only calculate once (not in ind and preach)

    Nft id_ind {create_identity(ind)};

    Nft v_id {compose(abstraction_framework, id_ind, 0, 0)};
    Nft product {compose(v_id, v_complement)};
    return mata::nft::complement(product, alphabet);
}


