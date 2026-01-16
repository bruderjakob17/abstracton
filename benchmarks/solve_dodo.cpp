#include "DodoParser.h"
#include <abstracton/interpretations.hpp>
#include <abstracton/abstracton.hpp>
#include <abstracton/utils/utils.hpp>
#include <abstracton/mata_extensions.hpp>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "SYNOPSIS\n";
        std::cout << "\tsolve_dodo FILENAME [ITYPE] [OPTIONS]\n";
        std::cout << "ARGUMENTS\n";
        std::cout << "\tFILENAME: path to filename of dodo problem instance\n";
        std::cout << "\tITYPE: which interpretation to use:\n";
        std::cout << "\t\tt: trap\n";
        std::cout << "\t\ts: siphon\n";
        std::cout << "\t\tf: flow\n";
        std::cout << "\t...if no ITYPE is given, the algorithm is run for all of the above.\n";
        std::cout << "\tOPTIONS\n";
        std::cout << "\t\t-v NUM\n";
        std::cout << "\t\t\tsets verbosity to NUM, which may be any of the following four:\n";
        std::cout << "\t\t\t  0: QUIET\n";
        std::cout << "\t\t\t  1: NORMAL\n";
        std::cout << "\t\t\t  2: VERBOSE\n";
        std::cout << "\t\t\t  3: DEBUG\n";
        return 0;
    }
    std::vector<enum SetInterpretation> interpretations{};
    if (argc == 2) {
        interpretations.push_back(Trap);
        interpretations.push_back(Siphon);
        interpretations.push_back(Flow);
    } else {
        // no switch for strings in c++...so if else branches:
        if (argv[2] == "t") {
            interpretations.push_back(Trap);
        } else if (argv[2] == "s") {
            interpretations.push_back(Siphon);
        } else if (argv[2] == "f") {
            interpretations.push_back(Flow);
        } else {
            std::cout << "unknown argument \"" << argv[2] << "\"\n";
            return 0;
        }
    }
    std::string filename{argv[1]};
    // DodoParserResult dpr = parseDodoJSON("dodo/token-passing.json");
    DodoParserResult dpr = parseDodoJSON(filename);

    std::cout << "parsed file " << filename << ". Result:" << std::endl;
    std::cout << "alphabet: " << dpr.string_alphabet->get_alphabet_symbols() << std::endl;
    assert(dpr.string_alphabet->is_equal(dpr.transitionRelation.alphabet));
    std::cout << "dpr transition rel:" << std::endl;
    std::cout << dpr.transitionRelation.print_to_dot();

    for (enum SetInterpretation itype : interpretations) {
        // build interpretation
        std::pair<mata::nft::Nft, std::shared_ptr<mata::OnTheFlyAlphabet>> ipa = trapInterpretation(dpr.string_alphabet.get(), itype);
        mata::nft::Nft interpretation = ipa.first;
        std::shared_ptr<mata::OnTheFlyAlphabet> powerset_alphabet_ptr = ipa.second;
        std::cout << interpretation.print_to_dot() << std::endl;

        // compute ind
        std::cout << "transition relation:" << std::endl;
        std::cout << dpr.transitionRelation.print_to_dot() << std::endl;
        std::cout << "computing ind" << std::endl;
        mata::nfa::Nfa ind {compute_ind(interpretation, dpr.transitionRelation, *dpr.string_alphabet, *powerset_alphabet_ptr, true)};
        ind = mata::nfa::minimize(ind);
        std::cout << "done" << std::endl;
        std::cout << ind.print_to_dot() << std::endl;

        // compute preach TODO update compute_ind and compute_preach to directly use the alphabets of the involved automata
        std::cout << "computing preach" << std::endl;
        mata::nft::Nft preach {compute_preach(interpretation, dpr.transitionRelation, *dpr.string_alphabet, *powerset_alphabet_ptr, std::make_optional<const mata::nfa::Nfa>(ind))};
        std::cout << "done" << std::endl;
        std::cout << preach.print_to_dot() << std::endl;
        size_t prev_size = preach.num_of_states();
        std::cout << "trimming..." << std::endl;
        preach = preach.trim();
        std::cout << "minimizing..." << std::endl;
        preach = mata::ext::minimize(preach);
        std::cout << "done. Reduced number of states from " << prev_size << " to " << preach.num_of_states() << std::endl;
        std::cout << "new automaton:\n";
        std::cout << preach.print_to_dot() << std::endl;

        // check whether safety can be proven, i.e. whether abstract safety holds
        std::cout << "trying to prove abstract safety..." << std::endl;
        std::cout << vec_to_string(check_abstract_safety(dpr.initialConfig, preach, dpr.properties)) << std::endl;
    }

    return 0;
}
