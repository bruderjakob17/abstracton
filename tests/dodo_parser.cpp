#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

#include <mata/alphabet.hh>
#include <mata/nft/nft.hh>

#include "DodoParser.h"

using namespace mata;
using namespace mata::nft;

// Regression test: the parser used to call Nft::add_transition() with a per-level symbol vector. In current MATA,
// add_transition() only takes a symbol *name* as a string, so the vector silently converted to a std::string and one
// bogus symbol per distinct transducer letter got appended to the OnTheFlyAlphabet.
TEST_CASE("Dodo parser keeps the concrete alphabet of the benchmark", "[dodo_parser]") {
    auto alphabet = std::make_shared<mata::OnTheFlyAlphabet>(std::vector<std::string>{ "t", "n" });

    DodoParserResult dpr {parseDodoJSON(
        std::string(ABSTRACTON_BENCHMARKS_DIR) + "/dodo/token-passing.json", logging::VerbosityLevel::QUIET, true
    )};

    // no symbol beyond "t" and "n" may be added to the alphabet while parsing
    const utils::OrdVector<Symbol> concrete_symbols {alphabet->translate_symb("t"), alphabet->translate_symb("n")};
    CHECK(dpr.string_alphabet->get_alphabet_symbols() == concrete_symbols);

    // one symbol per level, i.e. no transition spanning both levels at once
    CHECK_FALSE(dpr.transitionRelation.contains_jump_transitions());

    // transducer of token-passing.json: 0 -(t,n)-> 1, 0 -(n,n)-> 0, 1 -(n,t)-> 2, 2 -(n,n)-> 2
    Nft expected {Nft::with_levels(
        2, 3, utils::SparseSet<State>{ 0 }, utils::SparseSet<State>{ 2 },
        std::make_shared<mata::AlphabetLevels>(alphabet)
    )};
    const Symbol t {alphabet->translate_symb("t")};
    const Symbol n {alphabet->translate_symb("n")};
    expected.add_transition_by_levels(0, { t, n }, 1);
    expected.add_transition_by_levels(0, { n, n }, 0);
    expected.add_transition_by_levels(1, { n, t }, 2);
    expected.add_transition_by_levels(2, { n, n }, 2);

    CHECK(dpr.transitionRelation.is_identical(expected));
}