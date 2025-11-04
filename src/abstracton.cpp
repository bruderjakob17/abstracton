#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>
#include <abstracton/mata_extensions.hpp>

using namespace mata;
using namespace mata::nfa;
using namespace mata::nft;

// input: DETERMINISTIC abstraction framework!
// TODO convert output to debug output
// TODO exclude all a where V(a) = emptyset (optional, but nicer...)
Nfa compute_ind(const Nft& abstraction_framework, const Nft& transition_relation, const Alphabet& alphabet) {
    // project_1(Id intersect (V delta complement(inverse(V)))), then complement
    std::cout << "1" << std::endl;
    Nft v_delta {compose(abstraction_framework, transition_relation)};
    std::cout << "abstraction framework:" << std::endl;
    std::cout << abstraction_framework.print_to_dot(true) << std::endl;
    std::cout << "transition relation:" << std::endl;
    std::cout << transition_relation.print_to_dot(true) << std::endl;
    std::cout << "v delta:" << std::endl;
    std::cout << v_delta.print_to_dot(true) << std::endl;
    std::cout << "2" << std::endl;
    Nft v_complement {mata::ext::complement(abstraction_framework, alphabet)};
    std::cout << "complement of abstraction framework:" << std::endl;
    std::cout << v_complement.print_to_dot(true) << std::endl;
    Nft product1 {compose(v_delta, v_complement, 1, 1)};
    std::cout << "4" << std::endl;
    std::cout << "v delta v_complement^(-1):" << std::endl;
    std::cout << product1.print_to_dot(true) << std::endl;
    Nft preprojection {mata::nft::intersection(create_identity(alphabet), product1)};
    std::cout << "5" << std::endl;
    std::cout << "preprojection:" << std::endl;
    std::cout << preprojection.print_to_dot(true) << std::endl;
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


