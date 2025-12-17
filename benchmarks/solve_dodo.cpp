#include "DodoParser.h"
#include <abstracton/interpretations.hpp>

int main(int argc, char** argv) {
    // parseDodoJSON(argv[1]);
    DodoParserResult dpr = parseDodoJSON("dodo/token-passing.json");

    std::pair<mata::nft::Nft, mata::OnTheFlyAlphabet*> ipa = trapInterpretation(&dpr.string_alphabet, Flow);
    mata::nft::Nft interpretation = ipa.first;
    mata::OnTheFlyAlphabet* powerset_alphabet = ipa.second;
    std::unique_ptr<mata::OnTheFlyAlphabet> powerset_alphabet_ptr(powerset_alphabet);
    std::cout << interpretation.print_to_dot() << std::endl;

    return 0;
}
