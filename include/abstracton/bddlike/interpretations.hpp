#pragma once

/**
 * Example interpretations.
 */

#include <mata/nft/nft.hh>
#include <abstracton/interpretations.hpp> // for interpretation types

/**
 * @brief Creates a Transducer for the Trap/Flow/Siphon Interpretation.
 *
 * Give some alphabet S, let G be the powerset of S.
 * Then, the trap interpretation accepts some (v, w) in S* x G* iff there exists some i s.t. v_i in w_i.
 *
 * In contrast to the implementation in abstracton/interpretations.hpp, this implementation does not generate
 * a symbol for each element of the powerset G. Instead, a subset is represented as bit vector - see the file
 * abstracton/bddlike/bddlike_nft.hpp (SupersetVecAlphabet).
 *
 * @param[in] string_alphabet The alphabet S.
 * @return A transducer for the Trap Interpretation on S (first entry) and an interface for the superset alphabet (second entry).
 *
 * TODO: add to documentation siphon/flow capabilities of this function, rename function
 */
std::pair<mata::ext::BDDlikeNft, mata::ext::SupersetVecAlphabet> trapInterpretation(mata::OnTheFlyAlphabet* string_alphabet, enum SetInterpretation type);
