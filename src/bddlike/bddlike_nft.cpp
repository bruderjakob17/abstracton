#include <abstracton/bddlike/bddlike_nft.hpp>
#include <abstracton/mata_extensions.hpp>

void mata::ext::bddlike::BDDlikeNft::print_to_dot_using_alphabets(std::ostream &output) const {
    using namespace std;
    using namespace mata;
    using namespace mata::nft;
    using namespace mata::ext::bddlike;

    BoolVector is_state_drawn(num_of_states(), false);
    output << "digraph finiteAutomaton {" << std::endl
                 << "node [shape=circle];" << std::endl;

    // Double circle for final states
    for (const State final_state: final) {
        is_state_drawn[final_state] = true;
        assert(levels[final_state] == 0);
        output << final_state << " [shape=doublecircle];" << std::endl;
    }

    // Filter all states at level 0
    vector<State> states_at_level_0 {};
    for (State s{ 0 }; s < num_of_states(); ++s) {
        if (levels[s] == 0) {
            states_at_level_0.push_back(s);
        }
    }
    // build high-level transitions (i.e. transition sequences from level 0 to level 0)
    unordered_map<pair<State, State>, unordered_set<string>> labels;
    for (State source : states_at_level_0) {
        // build all reachable targets (first entry: how to get there, second entry: to which state)
        list<pair<vector<Symbol>, State>> worklist{};
        list<pair<vector<Symbol>, State>> targets{};

        for (const SymbolPost& sp : delta[source]) {
            for (const State& sp_target : sp.targets) {
                worklist.push_back(make_pair(vector<Symbol>{sp.symbol}, sp_target));
            }
        }

        while (!worklist.empty()) {
            auto partial_transition = *worklist.rbegin();
            worklist.pop_back();

            if (levels[partial_transition.second] == 0) {
                targets.push_back(partial_transition);
                continue;
            }

            for (const SymbolPost& sp : delta[partial_transition.second]) {
                for (const State& sp_target : sp.targets) {
                    // append sp.symbol to partial_transition.first
                    vector<Symbol> path{ partial_transition.first };
                    path.push_back(sp.symbol);
                    worklist.push_back(make_pair(path, sp_target));
                }
            }
        }

        for (auto transition : targets) {
            vector<Symbol> path = transition.first;
            State target = transition.second;

            // decode path into tuple of letters
            // TODO maybe collect arrows sharing same source and target?
            vector<vector<Symbol>> chunks{ split(path, alphabet_sizes) };
            assert(chunks.size() == alphabet_sizes.size());
            vector<string> letters{};
            for (int i{ 0 }; i < chunks.size(); ++i) {
                letters.push_back(alphabets[i]->print(chunks[i]));
            }
            labels[make_pair(source, target)].insert(vec_to_string(letters, ", ", "(", ")"));
        }
    }

    for (const auto& [st, st_labels] : labels) {
        vector<string> labels_vec(st_labels.begin(), st_labels.end());
        string long_label = vec_to_string(labels_vec, ", ", "", "");
        output << st.first << " -> " << st.second << " [label=\"" << long_label << "\", tooltip=\"" << long_label << "\"];" << std::endl;
    }

    // Circle for isolated states with no transitions
    for (State state : states_at_level_0) {
        if (!is_state_drawn[state]) {
            output << state << " [shape=circle];" << std::endl;
        }
    }

    // Arrow for initial states
    output << "node [shape=none, label=\"\"];" << std::endl;
    for (const State init_state: initial) {
        output << "i" << init_state << " -> " << init_state << ";" << std::endl;
    }

    output << "}" << std::endl;
}

std::string mata::ext::bddlike::BDDlikeNft::print_to_dot_using_alphabets() const {
    std::stringstream output;
    print_to_dot_using_alphabets(output);
    return output.str();
}

bool mata::ext::bddlike::BDDlikeNft::is_in_lang_by_levels(const std::vector<std::vector<std::vector<Symbol>>> level_words, bool match_prefix) const {
    // split each "big symbol" into its small symbols, and collect all small symbols at the same level
    std::vector<std::vector<std::vector<Symbol>>> rough_words{};
    for (const std::vector<std::vector<Symbol>>& high_level_word : level_words) {
        rough_words.push_back(zip(high_level_word));
    }
    std::vector<std::vector<Symbol>> flat_words{ flatten(rough_words) };
    return super::is_in_lang_by_levels(flat_words, match_prefix);
}

mata::ext::bddlike::BDDlikeNft mata::ext::bddlike::minimize(const mata::ext::bddlike::BDDlikeNft& aut) {
    auto alphabet_sizes = aut.alphabet_sizes;
    auto alphabets = aut.alphabets;
    mata::nft::Nft aut_as_nft{ aut.to_nft_copy() };
    mata::nft::Nft result_as_nft{mata::ext::minimize(aut_as_nft)}; // TODO move instead?
    mata::ext::bddlike::BDDlikeNft result{result_as_nft, alphabet_sizes, alphabets};
    return result;
}
