#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>
#include <abstracton/mata_extensions.hpp>
#include <abstracton/abstracton.hpp>
#include <abstracton/utils/utils.hpp>
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
    logging::logexp(logging::VerbosityLevel::VERBOSE, [&]() { return aut.print_to_dot(); }, verbosityLevel); \
}

using namespace mata;
using namespace mata::nfa;
using namespace mata::nft;

/// uses the fact that pi(Id cap (V delta V_comp_inv)) = pi_1((V delta) cap V_comp)
Nfa compute_ind_new(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, bool exclude_empty_abstractions, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    INIT_CLOCKS();

    using namespace std;
    using namespace mata::utils;

    TICK();
    Nft v_delta {compose(abstraction_framework, transition_relation)};
    TOCK("computing composition v_delta of abstraction framework and transition relation");
    PRINT_AUT("V o ->", v_delta);

    TICK();
    v_delta = mata::nft::reduce(v_delta);
    TOCK("reducing V o ->");
    PRINT_AUT("reduce(V o ->)", v_delta);

    std::vector<Alphabet*> alphabets {&abstract_alphabet, &concrete_alphabet};
    AlphabetLevels alphabet_levels(alphabets, AlphabetLevels::Mode::MultiLevel);
    TICK();
    Nft v_complement {mata::ext::complement(abstraction_framework, &alphabet_levels, true)};
    TOCK("computing complement v_complement of abstraction framework");
    PRINT_AUT("complement(V)", v_complement);

    TICK()
    v_complement = mata::ext::minimize(v_complement);
    TOCK("minimizing complement(V)");
    PRINT_AUT("min(complement(V))", v_complement);

    TICK();
    Nft preprojection {mata::nft::intersection(v_delta, v_complement)};
    TOCK("computing intersection 'preprojection' of v_delta and v_complement");
    PRINT_AUT("preprojection", preprojection);

    TICK();
    Nfa projection{ project(preprojection, 0) };
    TOCK("computing complement of ind (i.e. projecting preprojection onto first tape)");
    PRINT_AUT("complement(ind)", projection);

    TICK();
    projection = minimize_nfa(projection);
    TOCK("minimizing projection");
    PRINT_AUT("min(projection)", projection);

    TICK();
    Nfa ind{ mata::nfa::complement(projection, abstract_alphabet) };
    TOCK("computing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind has {} states", ind.num_of_states()), verbosityLevel);
    TICK();
    ind = minimize_nfa(ind);
    TOCK("minimizing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind has {} states", ind.num_of_states()), verbosityLevel);
    if (exclude_empty_abstractions) {
        // intersect with pi_1(V)
        logging::log(logging::VerbosityLevel::DEBUGV, "ind with empty abstractions:", verbosityLevel);
        logging::logexp(logging::VerbosityLevel::DEBUGV, [&]() { return ind.print_to_dot(); }, verbosityLevel);
        TICK();
        auto result = mata::nfa::intersection(ind, project(abstraction_framework, 0));
        TOCK("excluding empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        TICK();
        result = minimize_nfa(result);
        TOCK("minimizing ind without empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        return result;
    } else {
        return ind;
    }
}

/// previous function: compute_ind_old
/// this function improves upon the old implementation by explicitly constructing the product, instead of constructing a sequence of products
Nfa compute_ind(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, bool exclude_empty_abstractions, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    INIT_CLOCKS();

    using namespace std;
    using namespace mata::utils;

    TICK();
    Nfa result{};
    result.alphabet = &abstract_alphabet;

    unordered_map<vector<State>, State> product_state_to_state{};
    queue<vector<State>> worklist{};
    // initialize worklist by initial states of abstraction framework and transition relation
    for (const State& state0 : abstraction_framework.initial) {
        for (const State& state1 : transition_relation.initial) {
            for (const State& state2 : abstraction_framework.initial) {
                vector<State> init_state = {state0, state1, state2};
                worklist.push(init_state);
                State init_state_idx = result.add_state();
                result.initial.insert(init_state_idx);
                product_state_to_state[init_state] = init_state_idx;
            }
        }
    }

    optional<State> v_complement_trap_state = nullopt;

    while (!worklist.empty()) {
        vector<State> current_state = worklist.front();
        worklist.pop();
        State current_state_idx = product_state_to_state[current_state];

        State current_state_v_before = current_state[0];
        State current_state_transition = current_state[1];
        State current_state_v_after = current_state[2];

        for (const Symbol& abstract_symb : abstract_alphabet.get_alphabet_symbols()) {
            for (const SymbolPost& sp_0 : transition_relation.delta[current_state_transition]) {
                Symbol transition_letter_before = sp_0.symbol;
                for (const State& intermediate_state_transition : sp_0.targets) {
                    for (const SymbolPost& sp_1 : transition_relation.delta[intermediate_state_transition]) {
                        Symbol transition_letter_after = sp_1.symbol;
                        for (const State& next_state_transition : sp_1.targets) {
                            std::vector<Symbol> before_letter = {abstract_symb, transition_letter_before};
                            std::vector<Symbol> after_letter = {abstract_symb, transition_letter_after};

                            StateSet next_state_v_before_set = ext::traverse_symbol_by_levels(abstraction_framework, {current_state_v_before}, before_letter);
                            assert(next_state_v_before_set.size() <= 1);
                            if (next_state_v_before_set.size() == 0) {
                                continue;
                            }
                            State next_state_v_before = next_state_v_before_set.front();

                            State next_state_v_after;
                            bool next_state_v_after_is_final;
                            if (v_complement_trap_state.has_value() && current_state_v_after == v_complement_trap_state.value()) {
                                next_state_v_after = v_complement_trap_state.value();
                                next_state_v_after_is_final = true; // trap state in abstraction framework -> accepting in complement
                            } else {
                                StateSet next_state_v_after_set = ext::traverse_symbol_by_levels(abstraction_framework, {current_state_v_after}, after_letter);
                                assert(next_state_v_after_set.size() <= 1);
                                if (next_state_v_after_set.size() == 0) {
                                    if (!v_complement_trap_state.has_value()) {
                                        State new_trap_state = abstraction_framework.num_of_states();
                                        v_complement_trap_state = optional(new_trap_state);
                                    }
                                    next_state_v_after = v_complement_trap_state.value();
                                    next_state_v_after_is_final = true;
                                } else {
                                    next_state_v_after = next_state_v_after_set.front();
                                    next_state_v_after_is_final = !abstraction_framework.final.contains(next_state_v_after);
                                }
                            }

                            vector<State> next = {
                                next_state_v_before,
                                next_state_transition,
                                next_state_v_after
                            };
                            if (product_state_to_state.find(next) == product_state_to_state.end()) {
                                State next_state = result.add_state();
                                product_state_to_state[next] = next_state;
                                if (abstraction_framework.final.contains(next_state_v_before) &&
                                        transition_relation.final.contains(next_state_transition) &&
                                        next_state_v_after_is_final) {
                                    result.final.insert(next_state);
                                }
                                worklist.push(next);
                            }
                            result.delta.add(current_state_idx, abstract_symb, product_state_to_state[next]);
                        }
                    }
                }
            }
        }
    }
    TOCK("constructing complement of ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("complement of ind has {} states", result.num_of_states()), verbosityLevel);

    TICK();
    Nfa ind{ mata::nfa::complement(result, abstract_alphabet) };
    TOCK("computing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind has {} states", ind.num_of_states()), verbosityLevel);
    TICK();
    ind = minimize_nfa(ind);
    TOCK("minimizing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind has {} states", ind.num_of_states()), verbosityLevel);
    if (exclude_empty_abstractions) {
        // intersect with pi_1(V)
        logging::log(logging::VerbosityLevel::DEBUGV, "ind with empty abstractions:", verbosityLevel);
        logging::logexp(logging::VerbosityLevel::DEBUGV, [&]() { return ind.print_to_dot(); }, verbosityLevel);
        TICK();
        auto result = mata::nfa::intersection(ind, project(abstraction_framework, 0));
        TOCK("excluding empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        TICK();
        result = minimize_nfa(result);
        TOCK("minimizing ind without empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        return result;
    } else {
        return ind;
    }

    return ind;
}

// input: DETERMINISTIC abstraction framework!
// TODO convert output to debug output
// TODO exclude all a where V(a) = emptyset (optional, but nicer...)
Nfa compute_ind_old(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, bool exclude_empty_abstractions, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    // project_1(Id intersect (V delta complement(inverse(V)))), then complement
    INIT_CLOCKS();
    TICK();
    Nft v_delta {compose(abstraction_framework, transition_relation)};
    TOCK("computing composition v_delta of abstraction framework and transition relation");
    PRINT_AUT("v_delta", v_delta);

    TICK();
    v_delta = mata::nft::reduce(v_delta);
    TOCK("reducing v_delta");
    PRINT_AUT("reduce(v_delta)", v_delta);

    std::vector<Alphabet*> alphabets {&abstract_alphabet, &concrete_alphabet};
    AlphabetLevels alphabet_levels(alphabets, AlphabetLevels::Mode::MultiLevel);
    TICK();
    Nft v_complement {mata::ext::complement(abstraction_framework, &alphabet_levels, true)};
    TOCK("computing complement v_complement of abstraction framework");
    PRINT_AUT("v_complement", v_complement);

    TICK()
    v_complement = mata::ext::minimize(v_complement);
    TOCK("minimizing complement(V)");
    PRINT_AUT("min(complement(V))", v_complement);

    TICK();
    Nft product1 {compose(v_delta, v_complement, 1, 1)};
    TOCK("computing product product1 of v_delta with v_complement");
    PRINT_AUT("product1", product1);

    product1.alphabet = &abstract_alphabet;
    TICK();
    Nft preprojection {mata::nft::intersection(create_identity(abstract_alphabet), product1)};
    TOCK("computing identity on complement of ind (preprojection)");
    PRINT_AUT("preprojection", preprojection);

    TICK();
    Nfa projection{ project(preprojection, 0) };
    TOCK("computing complement of ind (i.e. projecting preprojection onto one tape)");
    PRINT_AUT("projection", projection);

    TICK();
    projection = minimize_nfa(projection);
    TOCK("minimizing projection");
    PRINT_AUT("min(projection)", projection);

    TICK();
    Nfa ind{ mata::nfa::complement(projection, abstract_alphabet) };
    TOCK("computing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind has {} states", ind.num_of_states()), verbosityLevel);
    TICK();
    ind = minimize_nfa(ind);
    TOCK("minimizing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind has {} states", ind.num_of_states()), verbosityLevel);
    if (exclude_empty_abstractions) {
        // intersect with pi_1(V)
        logging::log(logging::VerbosityLevel::DEBUGV, "ind with empty abstractions:", verbosityLevel);
        logging::logexp(logging::VerbosityLevel::DEBUGV, [&]() { return ind.print_to_dot(); }, verbosityLevel);
        TICK();
        auto result = mata::nfa::intersection(ind, project(abstraction_framework, 0));
        TOCK("excluding empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        TICK();
        result = minimize_nfa(result);
        TOCK("minimizing ind without empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        return result;
    } else {
        return ind;
    }
}
Nft compute_preach_complement(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, std::optional<const Nfa> ind, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    INIT_CLOCKS();

    // inverse(V) id_Ind complement(V), then complement
    Nfa ind_result;
    if (!ind.has_value()) {
        logging::log(logging::VerbosityLevel::DEBUGV, "ind not given as input to compute_preach_complement, need to construct.", verbosityLevel);
        ind_result = compute_ind(abstraction_framework, transition_relation, concrete_alphabet, abstract_alphabet, false, verbosityLevel, measure_time, no_dot_printing);
    } else {
        ind_result = ind.value();
    }

    std::vector<Alphabet*> alphabets {&abstract_alphabet, &concrete_alphabet};
    AlphabetLevels alphabet_levels(alphabets, AlphabetLevels::Mode::MultiLevel);
    Nft v_complement {mata::ext::complement(abstraction_framework, &alphabet_levels, true)}; // TODO only calculate once (not in ind and preach)

    TICK();
    Nft id_ind {create_identity(ind_result)};
    TOCK("constructing identity on ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("identity on ind has {} states", id_ind.num_of_states()), verbosityLevel);

    TICK();
    Nft v_id {compose(abstraction_framework, id_ind, 0, 0)};
    TOCK("computing product v_id of abstraction framework with identity on ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("v_id has {} states", v_id.num_of_states()), verbosityLevel);
    if (v_id.levels.num_of_levels != 2) {
        std::cout << "nft result of composition does not have 2 levels, need to handle.";
        throw 2;
    }
    TICK();
    Nft product {compose(v_id, v_complement)};
    TOCK("computing product of v_id with complement of abstraction framework");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("product has {} states", product.num_of_states()), verbosityLevel);
    product.alphabet = &concrete_alphabet;
    if (product.levels.num_of_levels != 2) {
        std::cout << "nft result of composition does not have 2 levels, need to handle.";
        throw 2;
    }

    return product;
}

Nft compute_preach(const Nft& abstraction_framework, const Nft& transition_relation, Alphabet& concrete_alphabet, Alphabet& abstract_alphabet, std::optional<const Nfa> ind, int verbosityLevel) {
    auto preach_comp = compute_preach_complement(abstraction_framework, transition_relation, concrete_alphabet, abstract_alphabet, ind, verbosityLevel);
    AlphabetLevels concrete_alphabet_levels({&concrete_alphabet}, AlphabetLevels::Mode::Global);
    return mata::ext::complement(preach_comp, &concrete_alphabet_levels);
}

std::vector<bool> check_abstract_safety_explicit(const mata::nfa::Nfa& initial_configurations, const mata::nft::Nft& preach, std::vector<mata::nfa::Nfa> unsafe_properties, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    INIT_CLOCKS();
    TICK();
    Nfa preach_image = apply(preach, initial_configurations);
    TOCK("calculating image of initial configs under preach");
    std::vector<bool> result{};
    for (const mata::nfa::Nfa& unsafe_property : unsafe_properties) {
        TICK();
        result.push_back(intersection(preach_image, unsafe_property).is_lang_empty());
        TOCK("calculating intersection of preach image with unsafe configs");
    }
    return result;
}

std::vector<bool> check_abstract_safety_lazy(const mata::nfa::Nfa& initial_configurations, const mata::nft::Nft& preach_complement, std::vector<mata::nfa::Nfa> unsafe_properties, Alphabet& concrete_alphabet, std::string universality_alg, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    INIT_CLOCKS();

    AlphabetLevels concrete_alphabet_levels(&concrete_alphabet);

    mata::nft::Nft initial_nft = mata::nft::builder::from_nfa_with_levels_advancing(initial_configurations, 1);
    std::vector<bool> result{};
    for (const mata::nfa::Nfa& unsafe_property : unsafe_properties) {
        TICK();
        mata::nft::Nft unsafe_property_nft = mata::nft::builder::from_nfa_with_levels_advancing(unsafe_property, 1);
        TOCK("constructing transducer for unsafe property");
        PRINT_AUT("unsafe property", unsafe_property_nft);

        TICK();
        mata::nft::Nft initial_unsafe_pairs = mata::ext::relational_product_length_preserving_dont_care({initial_nft, unsafe_property_nft});
        TOCK("constructing transducer for (initial, unsafe) pairs");
        PRINT_AUT("(initial, unsafe) pairs", initial_unsafe_pairs);

        TICK();
        mata::nft::Nft initial_unsafe_complement = mata::ext::complement(initial_unsafe_pairs, &concrete_alphabet_levels, true);
        TOCK("constructing transducer for complement of (initial, unsafe) pairs");
        PRINT_AUT("complement((initial, unsafe) pairs)", initial_unsafe_complement);

        TICK();
        mata::nft::Nft union2 = mata::nft::union_nondet(preach_complement, initial_unsafe_complement);
        TOCK("building union of complements of preach and (initial, unsafe)-transducer");
        Run cex; // possible counterexample, could e.g. be printed or returned... TODO
        // TODO select best algorithm here...
        if (universality_alg == "lazy") {
            TICK();
            result.push_back(mata::ext::is_universal_lazy(union2, &concrete_alphabet_levels, &cex, verbosityLevel, true));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else if (universality_alg == "lazy-bfs") {
            TICK();
            result.push_back(mata::ext::is_universal_lazy(union2, &concrete_alphabet_levels, &cex, verbosityLevel, false));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else if (universality_alg == "antichains-inclusion") {
            TICK();
            //TODO need not construct union2 and complement of initial-unsafe-pairs here
            result.push_back(mata::nft::is_included(initial_unsafe_pairs, preach_complement, &cex, &concrete_alphabet));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else if (universality_alg == "lazy-inclusion") {
            TICK();
            //TODO need not construct union2 and complement of initial-unsafe-pairs here
            result.push_back(mata::ext::is_included_lazy(initial_unsafe_pairs, preach_complement, &cex, verbosityLevel, true));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else if (universality_alg == "lazy-inclusion-bfs") {
            TICK();
            //TODO need not construct union2 and complement of initial-unsafe-pairs here
            result.push_back(mata::ext::is_included_lazy(initial_unsafe_pairs, preach_complement, &cex, verbosityLevel, false));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else if (universality_alg == "antichains-bfs") {
            TICK();
            result.push_back(mata::ext::is_universal_antichains(union2, &concrete_alphabet_levels, &cex, verbosityLevel, false));
            TOCK("checking universality using " + universality_alg + " algorithm");
        } else {
            if (universality_alg != "antichains") {
                logging::log(logging::VerbosityLevel::VERBOSE, "WARNING: unknown universality algorithm \"" + universality_alg + "\", defaulting to antichains.", verbosityLevel);
            }
            TICK();
            result.push_back(mata::ext::is_universal_antichains(union2, &concrete_alphabet_levels, &cex, verbosityLevel, true));
            TOCK("checking universality using " + universality_alg + " algorithm");
        }

        if (!result[result.size() - 1]) {
            logging::log(logging::VerbosityLevel::VERBOSE, "counterexample (abstractly reachable):", verbosityLevel);
            logging::logexp(logging::VerbosityLevel::VERBOSE, [&]() {
                std::vector<std::string> cex_symbols;
                cex_symbols.reserve(cex.word.size());
                for (const Symbol& x : cex.word) {
                    cex_symbols.push_back(concrete_alphabet.reverse_translate_symbol(x));
                }
                return vec_to_string(cex_symbols);
            }, verbosityLevel);
        }
    }
    return result;
}
