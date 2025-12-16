/**
 * Example interpretations.
 */

#include <mata/nft/nft.hh>

enum SetInterpretation {
    Trap,
    Siphon,
    Flow
};

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
 * @return A transducer for the Trap Interpretation on S.
 *
 * TODO: also allow other alphabets (e.g. IntAlphabet)
 * TODO: add to documentation siphon/flow capabilities of this function, rename function
 */
mata::nft::Nft trapInterpretation(mata::OnTheFlyAlphabet* string_alphabet, enum SetInterpretation type);
