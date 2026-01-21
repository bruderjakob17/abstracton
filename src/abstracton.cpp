#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>
#include <abstracton/mata_extensions.hpp>
#include <abstracton/abstracton.hpp>
#include <abstracton/utils/utils.hpp>

using namespace mata;
using namespace mata::nfa;
using namespace mata::nft;

// input: DETERMINISTIC abstraction framework!
// TODO convert output to debug output
// TODO exclude all a where V(a) = emptyset (optional, but nicer...)
Nfa compute_ind(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, bool exclude_empty_abstractions, int verbosityLevel) {
    // project_1(Id intersect (V delta complement(inverse(V)))), then complement
    Nft v_delta {compose(abstraction_framework, transition_relation)};
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
    std::vector<Alphabet*> alphabets {&abstract_alphabet, &concrete_alphabet};
    Nft v_complement {mata::ext::complement(abstraction_framework, nullptr, std::make_optional<std::vector<Alphabet*>>(alphabets), true)};
    logging::log(logging::VerbosityLevel::DEBUG, "complement of abstraction framework:", verbosityLevel);
    logging::logexp(logging::VerbosityLevel::DEBUG, [&]() { return v_complement.print_to_dot(); }, verbosityLevel);

    Nft product1 {compose(v_delta, v_complement, 1, 1)};
    if (product1.levels.num_of_levels != 2) {
        std::cout << "nft result of composition does not have 2 levels, need to handle.";
        throw 2;
    }
    product1.alphabet = &abstract_alphabet;
    Nft preprojection {mata::nft::intersection(create_identity(abstract_alphabet), product1)};
    Nfa projection{ project(preprojection, 0) };
    Nfa ind{ mata::nfa::complement(projection, abstract_alphabet) };
    if (exclude_empty_abstractions) {
        // intersect with pi_1(V)
        logging::log(logging::VerbosityLevel::DEBUG, "ind with empty abstractions:", verbosityLevel);
        logging::logexp(logging::VerbosityLevel::DEBUG, [&]() { return ind.print_to_dot(); }, verbosityLevel);
        return mata::nfa::intersection(ind, project(abstraction_framework, 0));
    } else {
        return ind;
    }
}
Nft compute_preach_complement(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, std::optional<const Nfa> ind, int verbosityLevel) {
    // inverse(V) id_Ind complement(V), then complement
    Nfa ind_result = ind.value_or(compute_ind(abstraction_framework, transition_relation, concrete_alphabet, abstract_alphabet, false, verbosityLevel));

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

Nft compute_preach(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, std::optional<const Nfa> ind, int verbosityLevel) {
    return mata::ext::complement(compute_preach_complement(abstraction_framework, transition_relation, concrete_alphabet, abstract_alphabet, ind, verbosityLevel), &concrete_alphabet);
}

std::vector<bool> check_abstract_safety(const mata::nfa::Nfa& initial_configurations, const mata::nft::Nft& preach, std::vector<mata::nfa::Nfa> unsafe_properties, int verbosityLevel) {
    Nfa preach_image = apply(preach, initial_configurations);
    std::vector<bool> result{};
    for (const mata::nfa::Nfa& unsafe_property : unsafe_properties) {
        result.push_back(intersection(preach_image, unsafe_property).is_lang_empty());
    }
    return result;
}
