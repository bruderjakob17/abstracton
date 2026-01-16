#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>
#include <abstracton/mata_extensions.hpp>
#include <abstracton/abstracton.hpp>

using namespace mata;
using namespace mata::nfa;
using namespace mata::nft;

// input: DETERMINISTIC abstraction framework!
// TODO convert output to debug output
// TODO exclude all a where V(a) = emptyset (optional, but nicer...)
Nfa compute_ind(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, bool exclude_empty_abstractions) {
    // project_1(Id intersect (V delta complement(inverse(V)))), then complement
    std::cout << "1" << std::endl;
    Nft v_delta {compose(abstraction_framework, transition_relation)};
    std::cout << "abstraction framework:" << std::endl;
    std::cout << abstraction_framework.print_to_dot() << std::endl;
    std::cout << "transition relation:" << std::endl;
    std::cout << transition_relation.print_to_dot() << std::endl;
    std::cout << "v delta:" << std::endl;
    std::cout << v_delta.print_to_dot() << std::endl;
    if (v_delta.levels.num_of_levels != 2) {
        std::cout << "compose function of nfts with ";
        std::cout << abstraction_framework.levels.num_of_levels;
        std::cout << ", ";
        std::cout << transition_relation.levels.num_of_levels;
        std::cout << " levels returned nft with ";
        std::cout << v_delta.levels.num_of_levels;
        std::cout << " levels." << std::endl;
        if (v_delta.is_lang_empty()) {
            std::cout << "language of nft result is empty.\n";
            // complement of ind is empty, ind is sigma star
            Nfa ind{ mata::nfa::builder::create_sigma_star_nfa(&abstract_alphabet) };
            std::cout << "sigma star nfa:\n";
            std::cout << ind.print_to_dot() << std::endl;
            if (exclude_empty_abstractions) {
                // intersect with pi_1(V)
                return mata::nfa::intersection(ind, project(abstraction_framework, 0));
            } else {
                return ind;
            }

        } else {
            std::cout << "language of nft result is not empty. Don't know what to do. Please inspect result NFT.\n";
            throw 2;
        }
    }
    std::cout << "2" << std::endl;
    std::vector<Alphabet*> alphabets {&abstract_alphabet, &concrete_alphabet};
    Nft v_complement {mata::ext::complement(abstraction_framework, nullptr, std::make_optional<std::vector<Alphabet*>>(alphabets), true)};
    std::cout << "complement of abstraction framework:" << std::endl;
    std::cout << v_complement.print_to_dot() << std::endl;
    Nft product1 {compose(v_delta, v_complement, 1, 1)};
    if (product1.levels.num_of_levels != 2) {
        std::cout << "nft result of composition does not have 2 levels, need to handle.";
        throw 2;
    }
    // TODO also preserve alphabets members in compose in mata
    product1.alphabet = &abstract_alphabet;
    std::cout << "4" << std::endl;
    std::cout << "v delta v_complement^(-1):" << std::endl;
    std::cout << product1.print_to_dot() << std::endl;
    Nft preprojection {mata::nft::intersection(create_identity(abstract_alphabet), product1)};
    std::cout << "5" << std::endl;
    std::cout << "preprojection:" << std::endl;
    std::cout << preprojection.print_to_dot() << std::endl;
    std::cout << "projection:" << std::endl;
    Nfa projection{ project(preprojection, 0) };
    std::cout << projection.print_to_dot() << std::endl;
    Nfa ind{ mata::nfa::complement(projection, abstract_alphabet) };
    if (exclude_empty_abstractions) {
        // intersect with pi_1(V)
        return mata::nfa::intersection(ind, project(abstraction_framework, 0));
    } else {
        return ind;
    }
}
Nft compute_preach_complement(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, std::optional<const Nfa> ind) {
    // inverse(V) id_Ind complement(V), then complement
    Nfa ind_result = ind.value_or(compute_ind(abstraction_framework, transition_relation, concrete_alphabet, abstract_alphabet));

    std::vector<Alphabet*> alphabets {&abstract_alphabet, &concrete_alphabet};
    Nft v_complement {mata::ext::complement(abstraction_framework, nullptr, std::make_optional<std::vector<Alphabet*>>(alphabets), true)}; // TODO only calculate once (not in ind and preach)

    Nft id_ind {create_identity(ind_result)};

    Nft v_id {compose(abstraction_framework, id_ind, 0, 0)};
    if (v_id.levels.num_of_levels != 2) {
        std::cout << "nft result of composition does not have 2 levels, need to handle.";
        throw 2;
    }
    Nft product {compose(v_id, v_complement)};
    product.alphabet = &concrete_alphabet;
    if (product.levels.num_of_levels != 2) {
        std::cout << "nft result of composition does not have 2 levels, need to handle.";
        throw 2;
    }

    return product;
}

Nft compute_preach(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, std::optional<const Nfa> ind) {
    return mata::ext::complement(compute_preach_complement(abstraction_framework, transition_relation, concrete_alphabet, abstract_alphabet, ind), &concrete_alphabet);
}

std::vector<bool> check_abstract_safety(const mata::nfa::Nfa& initial_configurations, const mata::nft::Nft& preach, std::vector<mata::nfa::Nfa> unsafe_properties) {
    Nfa preach_image = apply(preach, initial_configurations);
    std::vector<bool> result{};
    for (const mata::nfa::Nfa& unsafe_property : unsafe_properties) {
        result.push_back(intersection(preach_image, unsafe_property).is_lang_empty());
    }
    return result;
}
