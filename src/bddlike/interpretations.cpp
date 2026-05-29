#include <mata/nft/nft.hh>
#include <abstracton/utils/utils.hpp>
#include <abstracton/interpretations.hpp>
#include <abstracton/mata_extensions.hpp>
#include <abstracton/bddlike/interpretations.hpp>

std::pair<mata::ext::bddlike::BDDlikeNft, mata::ext::bddlike::PowersetVecAlphabet> mata::ext::bddlike::trapInterpretation(std::shared_ptr<mata::OnTheFlyAlphabet> string_alphabet, enum SetInterpretation type) {
    using namespace mata::ext;
    using namespace std;

    mata::utils::OrdVector<mata::Symbol> alphabet = string_alphabet->get_alphabet_symbols();
    /*std::vector<std::string> string_alphabet_vector {};

    for (mata::Symbol x : alphabet) {
        string_alphabet_vector.push_back(string_alphabet->reverse_translate_symbol(x));
    }*/

    PowersetVecAlphabet powerset_alphabet {string_alphabet};

    shared_ptr<SimpleVecAlphabet> string_alphabet_ptr = make_shared<SimpleVecAlphabet>(string_alphabet);
    shared_ptr<PowersetVecAlphabet> powerset_alphabet_ptr = make_shared<PowersetVecAlphabet>(powerset_alphabet);
    vector<shared_ptr<VecAlphabetPrinter>> alphabets {string_alphabet_ptr, powerset_alphabet_ptr};

    BDDlikeNft result = BDDlikeNft::with_alphabet_sizes({1, alphabet.size()}, 1, {0}, {}, alphabets);

    // inserts all pairs (v, I) where v (not) in I from state source to state target
    // positive means if v is in I, !positive means if v is not in I the corresponding pair (v, I) is inserted as edge
    auto insert_transitions = [&] (mata::nft::State source, mata::nft::State target, bool positive) {
        // create |S| * |S| auxiliary states and all necessary transitions
        for (mata::Symbol i{ 0 }; i < alphabet.size(); ++i) {
            // insert chain of length |S| for symbol i
            mata::nft::State current{ result.add_state_with_level(1) };
            result.delta.add(source, i, current);

            for (int j{ 0 }; j < alphabet.size(); ++j) {
                // determine next state
                mata::nft::State next = (j + 1 == alphabet.size()) ? target : result.add_state_with_level(j + 2);

                // check what transitions to add
                if (i == j) {
                    result.delta.add(current, positive ? 1 : 0, next);
                } else {
                    result.delta.add(current, 0, next);
                    result.delta.add(current, 1, next);
                }

                // update current state
                current = next;
            }
        }
    };

    vector<mata::nft::State> states{0};

    if (type != Siphon) {
        states.push_back(result.add_state());
        // add q0 -H-> q1
        insert_transitions(states[0], states[1], true);
    }

    // add q -M-> q
    for (const mata::nft::State& q : states) {
        insert_transitions(q, q, false);
    }

    // add final states
    if (type == Trap || type == Flow) {
        result.final.insert(states[1]);
    } else if (type == Siphon) {
        result.final.insert(states[0]);
    }

    return make_pair(result, powerset_alphabet);

    /*
    // construct automaton
    // states:
    // siphon: only q0
    // trap, flow: q0, q1
    // transitions:
    // each state: q -M-> q
    // q0 -H-> q1
    // only trap: q1 -H-> q1
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
                result.add_transition(states[0], {powerset_OnTheFlyAlphabet->translate_symb(powerset_alphabet[i]), string_alphabet->translate_symb(x)}, states[1]);
                // add q1 -H-> q1 if trap interpretation
                if (type == Trap) {
                    result.add_transition(states[1], {powerset_OnTheFlyAlphabet->translate_symb(powerset_alphabet[i]), string_alphabet->translate_symb(x)}, states[1]);
                }
            }
        }
    }
    // add transitions q -M-> q
    for (mata::nft::State q : states) {
        for (int i = 0; i < powerset.size(); ++i) {
            for (std::string x : powerset[i]) {
                result.add_transition(q, {powerset_OnTheFlyAlphabet->translate_symb(powerset_alphabet[i]), string_alphabet->translate_symb(x)}, q);
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
    */
}
