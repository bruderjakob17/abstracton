#pragma once

/**
 * Example interpretations.
 */

#include <mata/nft/nft.hh>
#include <abstracton/interpretations.hpp> // for interpretation types
#include <abstracton/bddlike/bddlike_nft.hpp>

namespace mata::ext::bddlike {

/**
 * @brief Creates a Transducer for the Trap/Flow/Siphon Interpretation.
 *
 * Give some alphabet S, let G be the powerset of S.
 * Then, the trap interpretation accepts some (v, w) in S* x G* iff there exists some i s.t. v_i in w_i.
 *
 * In contrast to the implementation in abstracton/interpretations.hpp, this implementation does not generate
 * a symbol for each element of the powerset G. Instead, a subset is represented as bit vector - see the file
 * abstracton/bddlike/bddlike_nft.hpp (PowersetVecAlphabet).
 *
 * NOTE: Also in contrast to the implementation in abstracton/interpretations.hpp, this implementation uses a
 * different order of tapes: (concrete, abstract), i.e. the subsets are read on the SECOND "tape", while the
 * elements are read on the FIRST tape.
 * This order leads to an automaton size of Theta(|S| * |S|) instead of Theta(2 ^ |S|).
 *
 * @param[in] string_alphabet The alphabet S.
 * @return A transducer for the Trap Interpretation on S. The alphabets created are referenced to by pointers managed by the transducer.
 *
 * TODO: add to documentation siphon/flow capabilities of this function, rename function
 */
BDDlikeNft trapInterpretation(std::shared_ptr<mata::OnTheFlyAlphabet> string_alphabet, enum SetInterpretation type);

}
