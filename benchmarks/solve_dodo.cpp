#include "DodoParser.h"
#include <abstracton/interpretations.hpp>

int main(int argc, char** argv) {
    // parseDodoJSON(argv[1]);
    DodoParserResult dpr = parseDodoJSON("dodo/token-passing.json");

    mata::nft::Nft interpretation = trapInterpretation(&dpr.string_alphabet, Flow);
    std::cout << interpretation.print_to_dot() << std::endl;

    return 0;
}
