#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>
#include <abstracton/mata_extensions.hpp>
#include <abstracton/abstracton.hpp>
#include <abstracton/utils/utils.hpp>
#include <abstracton/bddlike/bddlike_nft.hpp>
#include <abstracton/bddlike/interpretations.hpp>
#include <abstracton/bddlike/abstracton.hpp>
#include <format>
#include <queue>

#define INIT_CLOCKS() std::chrono::steady_clock::time_point begin, end;
#define TICK() if (measure_time) { begin = std::chrono::steady_clock::now(); }
#define TOCK(message) if (measure_time) { end = std::chrono::steady_clock::now(); std::cout << "Time needed for " << message << ": " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl; }

#define PRINT_AUT(name, aut) \
logging::log(logging::VerbosityLevel::VERBOSE, std::format("automaton {}:", name), verbosityLevel); \
if (no_dot_printing) { \
    logging::logexp(logging::VerbosityLevel::VERBOSE, [&]() { return std::format("{} states", aut.num_of_states()); }, verbosityLevel); \
} else { \
    logging::logexp(logging::VerbosityLevel::VERBOSE, [&]() { return aut.print_to_dot_using_alphabets(); }, verbosityLevel); \
}

using namespace mata;
using namespace mata::ext::bddlike;

/// uses the fact that ind = pi_2((inv(delta) V) cap V_comp)
mata::ext::bddlike::BDDlikeNft compute_ind_new(mata::ext::bddlike::BDDlikeNft& abstraction_framework, mata::ext::bddlike::BDDlikeNft& transition_relation, bool exclude_empty_abstractions, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    INIT_CLOCKS();

    using namespace std;
    using namespace mata::utils;

    TICK();
    BDDlikeNft delta_v {mata::ext::bddlike::compose(transition_relation, abstraction_framework, {0}, {0})};
    TOCK("computing composition v_delta of abstraction framework and transition relation");
    PRINT_AUT("inv(->) o V", delta_v);

    // TICK();
    // delta_v = mata::nft::reduce(delta_v);
    // TOCK("reducing inv(->) o V");
    // PRINT_AUT("reduce(inv(->) o V)", delta_v);

    TICK();
    BDDlikeNft v_complement {mata::ext::bddlike::complement(abstraction_framework, true)};
    TOCK("computing complement v_complement of abstraction framework");
    PRINT_AUT("complement(V)", v_complement);

    TICK()
    v_complement = mata::ext::bddlike::minimize(v_complement);
    TOCK("minimizing complement(V)");
    PRINT_AUT("min(complement(V))", v_complement);

    TICK();
    BDDlikeNft preprojection {mata::ext::bddlike::intersection(delta_v, v_complement)};
    TOCK("computing intersection 'preprojection' of delta_v and v_complement");
    PRINT_AUT("preprojection", preprojection);

    TICK();
    BDDlikeNft projection{ mata::ext::bddlike::project_to(preprojection, 1) };
    TOCK("computing complement of ind (i.e. projecting preprojection onto second tape)");
    PRINT_AUT("complement(ind)", projection);

    TICK();
    projection = mata::ext::bddlike::minimize(projection);
    TOCK("minimizing projection");
    PRINT_AUT("min(projection)", projection);

    TICK();
    BDDlikeNft ind{ mata::ext::bddlike::complement(projection) };
    TOCK("computing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind has {} states", ind.num_of_states()), verbosityLevel);
    TICK();
    ind = mata::ext::bddlike::minimize(ind);
    TOCK("minimizing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind has {} states", ind.num_of_states()), verbosityLevel);
    if (exclude_empty_abstractions) {
        // intersect with pi_1(V)
        logging::log(logging::VerbosityLevel::DEBUGV, "ind with empty abstractions:", verbosityLevel);
        logging::logexp(logging::VerbosityLevel::DEBUGV, [&]() { return ind.print_to_dot_using_alphabets(); }, verbosityLevel);
        TICK();
        auto result = mata::ext::bddlike::intersection(ind, mata::ext::bddlike::project_to(abstraction_framework, 1));
        TOCK("excluding empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        TICK();
        result = mata::ext::bddlike::minimize(result);
        TOCK("minimizing ind without empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        return result;
    } else {
        return ind;
    }


    // comment below: if abstraction framework has alphabets (abstract, concrete)

    /// uses the fact that pi(Id cap (V delta V_comp_inv)) = pi_1((V delta) cap V_comp)

    /*
    INIT_CLOCKS();

    using namespace std;
    using namespace mata::utils;

    TICK();
    BDDlikeNft v_delta {compose(abstraction_framework, transition_relation)};
    TOCK("computing composition v_delta of abstraction framework and transition relation");
    PRINT_AUT("V o ->", v_delta);

    // TICK();
    // v_delta = mata::nft::reduce(v_delta);
    // TOCK("reducing V o ->");
    // PRINT_AUT("reduce(V o ->)", v_delta);

    TICK();
    BDDlikeNft v_complement {mata::ext::bddlike::complement(abstraction_framework, true)};
    TOCK("computing complement v_complement of abstraction framework");
    PRINT_AUT("complement(V)", v_complement);

    TICK()
    v_complement = mata::ext::bddlike::minimize(v_complement);
    TOCK("minimizing complement(V)");
    PRINT_AUT("min(complement(V))", v_complement);

    TICK();
    BDDlikeNft preprojection {mata::ext::bddlike::intersection(v_delta, v_complement)};
    TOCK("computing intersection 'preprojection' of v_delta and v_complement");
    PRINT_AUT("preprojection", preprojection);

    TICK();
    BDDlikeNft projection{ mata::ext::bddlike::project_to(preprojection, 0) };
    TOCK("computing complement of ind (i.e. projecting preprojection onto first tape)");
    PRINT_AUT("complement(ind)", projection);

    TICK();
    projection = mata::ext::bddlike::minimize(projection);
    TOCK("minimizing projection");
    PRINT_AUT("min(projection)", projection);

    TICK();
    BDDlikeNft ind{ mata::ext::bddlike::complement(projection) };
    TOCK("computing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind has {} states", ind.num_of_states()), verbosityLevel);
    TICK();
    ind = mata::ext::bddlike::minimize(ind);
    TOCK("minimizing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind has {} states", ind.num_of_states()), verbosityLevel);
    if (exclude_empty_abstractions) {
        // intersect with pi_1(V)
        logging::log(logging::VerbosityLevel::DEBUGV, "ind with empty abstractions:", verbosityLevel);
        logging::logexp(logging::VerbosityLevel::DEBUGV, [&]() { return ind.print_to_dot(); }, verbosityLevel);
        TICK();
        auto result = mata::ext::bddlike::intersection(ind, mata::ext::bddlike::project_to(abstraction_framework, 0));
        TOCK("excluding empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        TICK();
        result = mata::ext::bddlike::minimize(result);
        TOCK("minimizing ind without empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        return result;
    } else {
        return ind;
    }
    */
}

mata::ext::bddlike::BDDlikeNft compute_preach_complement(mata::ext::bddlike::BDDlikeNft& abstraction_framework, mata::ext::bddlike::BDDlikeNft& transition_relation, std::optional<mata::ext::bddlike::BDDlikeNft>& ind, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    using namespace mata;
    using namespace mata::ext::bddlike;

    INIT_CLOCKS();

    // inverse(V) id_Ind complement(V), then complement
    BDDlikeNft ind_result;
    if (!ind.has_value()) {
        logging::log(logging::VerbosityLevel::DEBUGV, "ind not given as input to compute_preach_complement, need to construct.", verbosityLevel);
        ind_result = compute_ind_new(abstraction_framework, transition_relation, false, verbosityLevel, measure_time, no_dot_printing);
    } else {
        ind_result = ind.value();
    }


    TICK();
    BDDlikeNft v_complement {mata::ext::bddlike::complement(abstraction_framework, true)};
    TOCK("computing complement v_complement of abstraction framework");
    PRINT_AUT("complement(V)", v_complement); // TODO only calculate once (not in ind and preach)

    TICK();
    BDDlikeNft v_id {mata::ext::bddlike::compose(abstraction_framework, ind_result, {1}, {0}, false)};
    TOCK("computing product v_id of abstraction framework with ind");
    PRINT_AUT("v_id", v_id);
    // logging::log(logging::VerbosityLevel::VERBOSE, std::format("v_id has {} states", v_id.num_of_states()), verbosityLevel);
    TICK();
    BDDlikeNft product {mata::ext::bddlike::compose(v_id, v_complement, {1}, {1}, true)};
    TOCK("computing product of v_id with complement of abstraction framework");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("product has {} states", product.num_of_states()), verbosityLevel);
    PRINT_AUT("product", product);

    return product;
}

std::vector<bool> check_abstract_safety_explicit(mata::ext::bddlike::BDDlikeNft& initial_configurations, mata::ext::bddlike::BDDlikeNft& preach_complement, std::vector<mata::ext::bddlike::BDDlikeNft> unsafe_properties, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    INIT_CLOCKS();

    using namespace mata::ext::bddlike;

    TICK();
    BDDlikeNft preach = complement(preach_complement, true);
    TOCK("complementing complement of preach to obtain preach");
    PRINT_AUT("preach", preach)

    TICK();
    BDDlikeNft preach_image = mata::ext::bddlike::compose(initial_configurations, preach, {0}, {0});
    TOCK("calculating image of initial configs under preach");
    std::vector<bool> result{};
    for (BDDlikeNft& unsafe_property : unsafe_properties) {
        TICK();
        result.push_back(intersection(preach_image, unsafe_property).is_lang_empty());
        TOCK("calculating intersection of preach image with unsafe configs");
    }
    return result;
}
std::vector<bool> check_abstract_safety_lazy(mata::ext::bddlike::BDDlikeNft& initial_configurations, mata::ext::bddlike::BDDlikeNft& preach_complement, std::vector<mata::ext::bddlike::BDDlikeNft> unsafe_properties, std::string universality_alg, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    INIT_CLOCKS();

    using namespace mata::ext::bddlike;

    std::vector<bool> result{};
    for (BDDlikeNft& unsafe_property : unsafe_properties) {
        TICK();
        BDDlikeNft initial_unsafe_pairs = mata::ext::bddlike::relational_product_length_preserving_dont_care({initial_configurations, unsafe_property});
        TOCK("constructing transducer for (initial, unsafe) pairs");
        PRINT_AUT("(initial, unsafe) pairs", initial_unsafe_pairs);

        TICK();
        BDDlikeNft initial_unsafe_complement = mata::ext::bddlike::complement(initial_unsafe_pairs, true);
        TOCK("constructing transducer for complement of (initial, unsafe) pairs");
        PRINT_AUT("complement((initial, unsafe) pairs)", initial_unsafe_complement);

        TICK();
        BDDlikeNft union2 = mata::ext::bddlike::union_nondet(preach_complement, initial_unsafe_complement);
        TOCK("building union of complements of preach and (initial, unsafe)-transducer");
        mata::nft::Run cex; // possible counterexample, could e.g. be printed or returned... TODO
        // TODO select best algorithm here...
        if (universality_alg == "lazy") {
            TICK();
            result.push_back(mata::ext::bddlike::is_universal_lazy(union2, &cex, verbosityLevel, true));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else if (universality_alg == "lazy-bfs") {
            TICK();
            result.push_back(mata::ext::bddlike::is_universal_lazy(union2, &cex, verbosityLevel, false));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else if (universality_alg == "antichains-inclusion") {
            TICK();
            //TODO need not construct union2 and complement of initial-unsafe-pairs here
            result.push_back(mata::ext::bddlike::is_included_antichains(initial_unsafe_pairs, preach_complement, &cex, verbosityLevel, true));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else if (universality_alg == "lazy-inclusion") {
            TICK();
            //TODO need not construct union2 and complement of initial-unsafe-pairs here
            result.push_back(mata::ext::bddlike::is_included_lazy(initial_unsafe_pairs, preach_complement, &cex, verbosityLevel, true));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else if (universality_alg == "lazy-inclusion-bfs") {
            TICK();
            //TODO need not construct union2 and complement of initial-unsafe-pairs here
            result.push_back(mata::ext::bddlike::is_included_lazy(initial_unsafe_pairs, preach_complement, &cex, verbosityLevel, false));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else if (universality_alg == "antichains-bfs") {
            TICK();
            result.push_back(mata::ext::bddlike::is_universal_antichains(union2, &cex, verbosityLevel, false));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else {
            if (universality_alg != "antichains") {
                logging::log(logging::VerbosityLevel::VERBOSE, "WARNING: unknown universality algorithm \"" + universality_alg + "\", defaulting to antichains.", verbosityLevel);
            }
            TICK();
            result.push_back(mata::ext::bddlike::is_universal_antichains(union2, &cex, verbosityLevel, true));
            TOCK("checking universality using " + universality_alg + " algorithm");
        }

        if (!result[result.size() - 1]) {
            logging::log(logging::VerbosityLevel::VERBOSE, "counterexample (abstractly reachable):", verbosityLevel);
            logging::logexp(logging::VerbosityLevel::VERBOSE, [&]() {
                // std::vector<std::string> cex_symbols;
                // cex_symbols.reserve(cex.word.size());
                // for (const Symbol& x : cex.word) {
                //     cex_symbols.push_back(concrete_alphabet.reverse_translate_symbol(x));
                // }
                // return vec_to_string(cex_symbols);
                return "not implemented yet";
            }, verbosityLevel);
        }
    }
    return result;
}
