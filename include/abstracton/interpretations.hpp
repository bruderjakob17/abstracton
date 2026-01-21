#pragma once

/**
 * Example interpretations.
 */

#include <mata/nft/nft.hh>

enum SetInterpretation {
    Trap,
    Siphon,
    Flow
};

inline std::string to_string(SetInterpretation i) {
    switch (i) {
        case Trap: return "Trap";
        case Siphon: return "Siphon";
        case Flow: return "Flow";
        default: return "INVALID";
    }
}

/**
 * @brief Creates a Transducer for the Trap Interpretation.
 *
 * Give some alphabet S, let G be the powerset of S.
 * Then, the trap interpretation accepts some (v, w) in S* x G* iff there exists some i s.t. v_i in w_i.
 *
 * Since mata only allows strings as symbols (using OnTheFlyAlphabet), sets are encoded as strings:
 *
 * (ab, c, cx) encodes {ab, c, cx}
 *
 * The elements are ordered lexicographically in the string representation, making it unique.
 *
 * @param[in] string_alphabet The alphabet S.
 * @return A transducer for the Trap Interpretation on S, and a pointer to the constructed Alphabet for the power set of S (needs to be deconstructed eventually).
 *
 * TODO: also allow other alphabets (e.g. IntAlphabet)
 * TODO: add to documentation siphon/flow capabilities of this function, rename function
 */
std::pair<mata::nft::Nft, std::shared_ptr<mata::OnTheFlyAlphabet>> trapInterpretation(mata::OnTheFlyAlphabet* string_alphabet, enum SetInterpretation type);
