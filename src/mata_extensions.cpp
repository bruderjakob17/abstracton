#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>
#include <abstracton/mata_extensions.hpp>
#include <mata/nfa/algorithms.hh>
#include <abstracton/utils/utils.hpp>
#include <random>

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

namespace mata::ext {

    // TODO: implement functions that also move the nft to an nfa to save resources?

    Nft determinize(const Nft& nft) {
        int levels = nft.levels.num_of_levels;
        Nfa aut {nft.to_nfa_copy()};
        Nfa aut_det {determinize(aut)};
        return mata::nft::builder::from_nfa_with_levels_advancing(aut_det, levels);
    }

    Nft minimize(const Nft& nft) {
        int levels = nft.levels.num_of_levels;
        Nfa aut {nft.to_nfa_copy()};
        // TODO for mindet, brzozowski better?
        Nfa aut_det {determinize(aut)};
        Nfa aut_min {algorithms::minimize_hopcroft(aut_det)};
        return mata::nft::builder::from_nfa_with_levels_advancing(aut_min, levels);
    }

    void make_complete(mata::nft::Nft& nft, const mata::utils::OrdVector<Symbol>& symbols) {
        int levels = nft.levels.num_of_levels;

        // insert sink state (with multiple levels)
        std::vector<State> sinks {};
        for (int i = 0; i < levels; i++) {
            sinks.push_back(nft.add_state_with_level(i));
            std::cout << "adding sink state " << sinks[i] << " at level " << i << std::endl;
            std::cout << "sink state " << sinks[i] << " has level " << nft.levels[sinks[i]] << std::endl;
        }

        const size_t num_of_states{ nft.num_of_states() };

        for (State state{ 0 }; state < num_of_states; ++state) {
            mata::utils::OrdVector<Symbol> used_symbols {};
            for (const SymbolPost& symbol_post : nft.delta[state]) {
                used_symbols.insert(symbol_post.symbol);
            }
            const mata::utils::OrdVector<Symbol> unused_symbols{ symbols.difference(used_symbols) };
            const unsigned int state_level{ nft.levels[state] };
            const unsigned int next_level{ (state_level + 1) % levels };
            for (const Symbol symbol : unused_symbols) {
                // add to delta state -symbol-> sinks[i], iff state is in level i - 1
                nft.delta.add(state, symbol, sinks[next_level]);
            }
        }
    }

    Nft complement(const Nft& aut, const mata::utils::OrdVector<Symbol>& symbols, bool minimize_during_determinization) {
        Nft result;

        if (minimize_during_determinization) {
            result = mata::ext::minimize(aut);
        } else {
            result = mata::ext::determinize(aut);
        }

        make_complete(result, symbols);
        utils::SparseSet<State> new_final_states{};

        const size_t num_of_states{ result.num_of_states() };

        for (State state{ 0 }; state < num_of_states; ++state) {
            if (result.levels[state] == 0 && !result.final.contains(state)) {
                std::cout << "state " << state << " has level " << result.levels[state] << " and is not final" << std::endl;
                new_final_states.insert(state);
            }
        }

        result.final = new_final_states;
        return result;
    }

    Nft complement(const mata::nft::Nft& nft, const mata::Alphabet& alphabet, bool minimize_during_determinization) {
        // std::cout << "complementing the following nft" << std::endl;
        // std::cout << nft.print_to_dot(true) << std::endl;
        // Nft nft_comp {mata::nft::complement(nft, alphabet)};
        // std::cout << "result as nft:" << std::endl;
        // std::cout << nft_comp.print_to_dot(true) << std::endl;
        // int levels = nft.levels.num_of_levels;
        // Nfa aut {nft_comp.to_nfa_copy()};
        // std::cout << "result as nfa:" << std::endl;
        // std::cout << aut.print_to_dot(true) << std::endl;
        // return mata::nft::builder::from_nfa_with_levels_advancing(aut, levels);
        return complement(nft, alphabet.get_alphabet_symbols(), minimize_during_determinization);
    }

    void padding_closure(Nfa& nfa, Symbol padding_symbol) {
        // compute predecessors wrt. delta (restricted to padding_symbol)
        // -> construct graph with edges (i, j) meaning that (j, padding_symbol, i) is in delta
        std::unordered_map<State, std::unordered_set<State>> predecessors {};

        std::unordered_set<State> visited {};
        std::unordered_set<State> worklist {};

        for (State i : nfa.initial) {
            worklist.insert(i);
        }

        while (!worklist.empty()) {
            // pop element from worklist
            State q = *worklist.begin();
            worklist.erase(worklist.begin());
            visited.insert(q);

            // explore successors of q and add them to correct sets
            for (const SymbolPost& sp : nfa.delta[q]) {
                for (const State target : sp.targets) {
                    if (sp.symbol == padding_symbol) {
                        predecessors[target].insert(q);
                    }
                    if (!visited.contains(target)) {
                        worklist.insert(target);
                    }
                }
            }
        }

        // DEBUG output
        std::cout << "visited:" << std::endl;
        for (auto i : visited) {
            std::cout << i;
        }
        std::cout << std::endl;
        std::cout << "pred size " << predecessors.size() << std::endl;
        for (const auto& [key, values] : predecessors) {
            std::cout << key << " : { ";
            for (int v : values) {
                std::cout << v << " ";
            }
            std::cout << "}\n";
        }

        // dfs on predecessors to find states that need to be final
        visited.clear();
        worklist.clear();

        for (State i : nfa.final) {
            worklist.insert(i);
        }

        while (!worklist.empty()) {
            // pop element from worklist
            State q = *worklist.begin();
            worklist.erase(worklist.begin());
            visited.insert(q);

            // make q final in nfa
            nfa.final.insert(q);

            // explore predecessors of q
            for (const State& p : predecessors[q]) {
                if (!visited.contains(p)) {
                    worklist.insert(p);
                }
            }
        }
    }

    // code copied and adapted from mata's create_random_nfa_tabakov_vardi
    Nft builder::create_random_nft_tabakov_vardi(const size_t num_of_levels, const size_t num_of_states, const std::vector<size_t>& alphabet_sizes, const double states_trans_ratio_per_symbol, const double final_state_density, const std::optional<unsigned int> seed) {
        if (num_of_states == 0) {
            return Nft::with_levels(num_of_levels);
        }
        if (states_trans_ratio_per_symbol < 0 || static_cast<size_t>(states_trans_ratio_per_symbol) > num_of_states) {
            // Maximum of num_of_states^2 unique transitions for one symbol can be created.
            throw std::runtime_error("Transition density must be in range [0, num_of_states]");
        }
        if (final_state_density < 0 || final_state_density > 1) {
            // Maximum of num_of_states final states can be created.
            throw std::runtime_error("Final state density must be in range (0, 1]");
        }
        if (num_of_levels != alphabet_sizes.size()) {
            // did not specify alphabet size uniquely for each alphabet
            throw std::runtime_error("Must give exactly one alphabet size for each level");
        }

        Nft nft = Nft::with_levels(num_of_levels, num_of_states, { 0 }, { 0 }, new OnTheFlyAlphabet{});

        // Initialize the random number generator
        unsigned int seed_val = seed.value_or(std::random_device{}());
        std::mt19937 gen(seed_val); // Mersenne Twister engine

        // Unique final state generator
        std::vector<State> states(num_of_states);
        std::iota(states.begin(), states.end(), 0);
        std::shuffle(states.begin() + 1, states.end(), gen); // Starting from 1, because 0 is always final state.

        // Create final states
        const size_t num_of_final_states{ static_cast<size_t>(std::round(static_cast<double>(num_of_states) * final_state_density)) };
        for (size_t i = 0; i < num_of_final_states; ++i) {
            nft.final.insert(states[i]);
        }

        // Unique transition generator
        std::vector<State> one_dimensional_transition_matrix(num_of_states * num_of_states);
        std::iota(one_dimensional_transition_matrix.begin(), one_dimensional_transition_matrix.end(), 0);

        // Create transitions
        // Using std::min because, in some universe, casting and rounding might cause the number of transitions to exceed the number of possible transitions by 1
        // and then an access to the non-existing element of one_dimensional_transition_matrix would occur.
        const size_t num_of_transitions_per_symbol{ std::min(static_cast<size_t>(std::round(static_cast<double>(num_of_states) * states_trans_ratio_per_symbol)), one_dimensional_transition_matrix.size()) };
        std::vector<unsigned int> tuple_bound;
        tuple_bound.reserve(alphabet_sizes.size());
        for (auto x : alphabet_sizes)
            tuple_bound.push_back(static_cast<unsigned int>(x));

        for (auto symbol_vec : BoundedTuples(tuple_bound)) {
            std::shuffle(one_dimensional_transition_matrix.begin(), one_dimensional_transition_matrix.end(), gen);
            for (size_t i = 0; i < num_of_transitions_per_symbol; ++i) {
                const State source{ one_dimensional_transition_matrix[i] / num_of_states };
                const State target{ one_dimensional_transition_matrix[i] % num_of_states };
                nft.add_transition(source, symbol_vec, target);
            }
        }
        return nft;
    }
}
