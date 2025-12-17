#include <mata/nft/nft.hh>
#include <abstracton/utils/utils.hpp>
#include <abstracton/interpretations.hpp>
#include <abstracton/mata_extensions.hpp>

std::pair<mata::nft::Nft, mata::OnTheFlyAlphabet*> trapInterpretation(mata::OnTheFlyAlphabet* string_alphabet, enum SetInterpretation type) {

    mata::utils::OrdVector<mata::Symbol> alphabet = string_alphabet->get_alphabet_symbols();
    std::vector<std::string> string_alphabet_vector {};

    for (mata::Symbol x : alphabet) {
        string_alphabet_vector.push_back(string_alphabet->reverse_translate_symbol(x));
    }

    std::vector<std::string> powerset_alphabet {};
    std::vector<std::vector<std::string>> powerset {};

    for (std::vector<std::string> subset : SubsequenceVectors(string_alphabet_vector)) {
        std::string subset_symbol = vec_to_string(subset, ",");
        powerset_alphabet.push_back(subset_symbol);
        powerset.push_back(subset);
    }

    mata::OnTheFlyAlphabet* powerset_OnTheFlyAlphabet = new mata::OnTheFlyAlphabet(powerset_alphabet);

    std::vector<mata::Alphabet*> alphabets {string_alphabet, powerset_OnTheFlyAlphabet};

    mata::nft::Nft result = mata::nft::Nft::with_levels(2, 0, {}, {}, nullptr, alphabets);

    // construct automaton
    // states:
    // siphon: only q0
    // trap, flow: q0, q1
    // transitions:
    // each state: q -M-> q
    // q0 -H-> q1
    // where H: (v, I): v in I, M: (v, I): v not in I
    std::vector<mata::nft::State> states{};
    states.push_back(result.add_state());
    result.initial.insert(states[0]);
    if (type != Siphon) {
        states.push_back(result.add_state());
        // add q0 -H-> q1
        for (int i = 0; i < powerset.size(); ++i) {
            // compute complement of powerset[i]
            std::vector<std::string> subset_complement = vec_complement(powerset[i], string_alphabet_vector);
            for (std::string x : subset_complement) {
                result.add_transition(states[0], {string_alphabet->translate_symb(x), powerset_OnTheFlyAlphabet->translate_symb(powerset_alphabet[i])}, states[1]);
            }
        }
    }
    // add transitions q -M-> q
    for (mata::nft::State q : states) {
        for (int i = 0; i < powerset.size(); ++i) {
            for (std::string x : powerset[i]) {
                result.add_transition(q, {string_alphabet->translate_symb(x), powerset_OnTheFlyAlphabet->translate_symb(powerset_alphabet[i])}, q);
            }
        }
    }

    // add final states
    if (type == Trap || type == Flow) {
        result.final.insert(states[1]);
    } else if (type == Siphon) {
        result.final.insert(states[0]);
    }

    result = mata::ext::minimize(result);

    return std::make_pair(result, powerset_OnTheFlyAlphabet);
}
