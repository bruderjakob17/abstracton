#include "DodoParser.h"
#include <format>
#include <abstracton/interpretations.hpp>
#include <abstracton/abstracton.hpp>
#include <abstracton/utils/utils.hpp>
#include <abstracton/mata_extensions.hpp>

int main(int argc, char** argv) {
    using namespace logging;

    if (argc < 2) {
        std::cout << "SYNOPSIS\n";
        std::cout << "\tsolve_dodo FILENAME [OPTIONS]\n";
        std::cout << "ARGUMENTS\n";
        std::cout << "\tFILENAME: path to filename of dodo problem instance\n";
        std::cout << "\tOPTIONS\n";
        std::cout << "\t\t-i ITYPE: which interpretation to use:\n";
        std::cout << "\t\t\tt: trap\n";
        std::cout << "\t\t\ts: siphon\n";
        std::cout << "\t\t\tf: flow\n";
        std::cout << "\t\t...if no ITYPE is given, the algorithm is run for all of the above.\n";
        std::cout << "\t\t-v NUM\n";
        std::cout << "\t\t\tsets verbosity to NUM, which may be any of the following four:\n";
        std::cout << "\t\t\t  0: QUIET\n";
        std::cout << "\t\t\t  1: NORMAL\n";
        std::cout << "\t\t\t  2: VERBOSE\n";
        std::cout << "\t\t\t  3: DEBUG\n";
        std::cout << "\t\t-p PROPERTY\n";
        std::cout << "\t\t\tonly checks abstract safety for property PROPERTY\n";
        std::cout << "\t\t--minimize-input\n";
        std::cout << "\t\t\talso minimize input (automata for initial configurations, transition relation and unsafe configurations)\n";
        return 0;
    }
    std::vector<enum SetInterpretation> interpretations{};
    int verbosityLevel = VerbosityLevel::NORMAL;
    std::optional<std::string> property = std::nullopt;
    std::vector<std::string> propertyNames{};
    bool minimize_input = false;
    std::string filename{};
    // no switch for strings in c++...so if else branches:
    for (int i{ 1 }; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "-i" || arg == "--interpretation") {
            if (i + 1 < argc) {
                std::string itype(argv[i + 1]);
                ++i;
                if (itype == "t" || itype == "trap") {
                    interpretations.push_back(Trap);
                } else if (itype == "s" || itype == "siphon") {
                    interpretations.push_back(Siphon);
                } else if (itype == "f" || itype == "flow") {
                    interpretations.push_back(Flow);
                } else {
                    std::cout << "ITYPE must be any of the following: t, s, f\n";
                }
            } else {
                std::cout << "need to specify ITYPE after " << argv[i] << " option";
                std::cout << ", which must be any of the following: t, s, f\n";
            }
        } else if (arg == "-v" || arg == "--verbosity") {
            if (i + 1 < argc && atoi(argv[i + 1]) >= 0 && atoi(argv[i + 1]) <= 3) {
                verbosityLevel = atoi(argv[i + 1]);
                ++i;
            } else {
                std::cout << "need to give verbosity level 0, 1, 2 or 3 after " << argv[i] << " option\n";
                return 0;
            }
        } else if (arg == "-p" || arg == "--property") {
            if (i + 1 < argc) {
                property = std::make_optional(std::string(argv[i + 1]));
                ++i;
            } else {
                std::cout << "need to name property after " << argv[i] << " option\n";
                return 0;
            }
        } else if (arg == "--minimize-input") { // TODO change to --no-minimize-input instead?
            minimize_input = true;
        } else if (arg.find(".json") != std::string::npos) {
            filename = arg;
        } else {
            std::cout << "unknown argument \"" << argv[i] << "\"\n";
            return 0;
        }
    }
    if (interpretations.empty()) {
        log(VerbosityLevel::VERBOSE, "did not receive particular interpretation, defaulting to checking all three of t, s, f", verbosityLevel);
        interpretations = {Trap, Siphon, Flow};
    }
    // DodoParserResult dpr = parseDodoJSON("dodo/token-passing.json");
    DodoParserResult dpr = parseDodoJSON(filename, verbosityLevel);

    log(VerbosityLevel::VERBOSE, "parsed file " + filename + ".", verbosityLevel);
    log(VerbosityLevel::DEBUG, "alphabet: " + stream_to_string(dpr.string_alphabet->get_alphabet_symbols()), verbosityLevel);
    assert(dpr.string_alphabet->is_equal(dpr.transitionRelation.alphabet));
    log(VerbosityLevel::DEBUG, "initial configurations:", verbosityLevel);
    logexp(VerbosityLevel::DEBUG, [&]() { return dpr.initialConfig.print_to_dot(); }, verbosityLevel);
    log(VerbosityLevel::DEBUG, "transition relation:", verbosityLevel);
    logexp(VerbosityLevel::DEBUG, [&]() { return dpr.transitionRelation.print_to_dot(); }, verbosityLevel);
    assert(dpr.properties.size() == dpr.propertyNames.size());
    log(VerbosityLevel::DEBUG, "properties:", verbosityLevel);
    for (int i{ 0 }; i < dpr.properties.size(); ++i) {
        log(VerbosityLevel::DEBUG, dpr.propertyNames[i], verbosityLevel);
        logexp(VerbosityLevel::DEBUG, [&]() { return dpr.properties[i].print_to_dot(); }, verbosityLevel);
    }

    // TODO only minimize relevant input, e.g. don't minimize properties that should not be checked
    if (minimize_input) {
        log(VerbosityLevel::VERBOSE, "minimizing input...", verbosityLevel);

        log(VerbosityLevel::VERBOSE, std::format("automaton for initialConfig has {} states.", dpr.initialConfig.num_of_states()), verbosityLevel);
        dpr.initialConfig = minimize_nfa(dpr.initialConfig);
        log(VerbosityLevel::VERBOSE, std::format("minimized automaton for initialConfig has {} states.", dpr.initialConfig.num_of_states()), verbosityLevel);

        log(VerbosityLevel::VERBOSE, std::format("automaton for transitionRelation has {} states.", dpr.transitionRelation.num_of_states()), verbosityLevel);
        dpr.transitionRelation = mata::ext::minimize(dpr.transitionRelation);
        log(VerbosityLevel::VERBOSE, std::format("minimized automaton for transitionRelation has {} states.", dpr.transitionRelation.num_of_states()), verbosityLevel);

        for (int i{ 0 }; i < dpr.properties.size(); ++i) {
            if (!property.has_value() || property.value() == dpr.propertyNames[i]) {
                log(VerbosityLevel::VERBOSE, std::format("automaton for property {} has {} states.", dpr.propertyNames[i], dpr.properties[i].num_of_states()), verbosityLevel);
                dpr.properties[i] = minimize_nfa(dpr.properties[i]);
                log(VerbosityLevel::VERBOSE, std::format("minimized automaton for property {} has {} states.", dpr.propertyNames[i], dpr.properties[i].num_of_states()), verbosityLevel);
            }
        }

    }

    for (enum SetInterpretation itype : interpretations) {
        log(VerbosityLevel::QUIET, "using " + to_string(itype) + " interpretation", verbosityLevel);
        // build interpretation
        std::pair<mata::nft::Nft, std::shared_ptr<mata::OnTheFlyAlphabet>> ipa = trapInterpretation(dpr.string_alphabet.get(), itype);
        mata::nft::Nft interpretation = ipa.first;
        std::shared_ptr<mata::OnTheFlyAlphabet> powerset_alphabet_ptr = ipa.second;
        logexp(VerbosityLevel::DEBUG, [&]() { return interpretation.print_to_dot(); }, verbosityLevel);

        // compute ind
        log(VerbosityLevel::NORMAL, "computing ind...", verbosityLevel);

        mata::nfa::Nfa ind {compute_ind(interpretation, dpr.transitionRelation, *dpr.string_alphabet, *powerset_alphabet_ptr, true)};

        log(VerbosityLevel::NORMAL, std::format("automaton for ind has {} states.", ind.num_of_states()), verbosityLevel);

        ind = mata::nfa::minimize(ind);

        log(VerbosityLevel::NORMAL, std::format("minimized automaton for ind has {} states.", ind.num_of_states()), verbosityLevel);
        log(VerbosityLevel::NORMAL, "...done.", verbosityLevel);
        log(VerbosityLevel::VERBOSE, "automaton for ind:", verbosityLevel);
        logexp(VerbosityLevel::VERBOSE, [&]() { return ind.print_to_dot(); }, verbosityLevel);

        // compute preach
        log(VerbosityLevel::NORMAL, "computing preach...", verbosityLevel);

        mata::nft::Nft preach {compute_preach(interpretation, dpr.transitionRelation, *dpr.string_alphabet, *powerset_alphabet_ptr, std::make_optional<const mata::nfa::Nfa>(ind))};

        log(VerbosityLevel::NORMAL, std::format("automaton for preach has {} states.", preach.num_of_states()), verbosityLevel);

        preach = preach.trim();
        preach = mata::ext::minimize(preach);

        log(VerbosityLevel::NORMAL, std::format("minimized automaton for preach has {} states.", preach.num_of_states()), verbosityLevel);
        log(VerbosityLevel::NORMAL, "...done.", verbosityLevel);
        log(VerbosityLevel::VERBOSE, "automaton for preach:", verbosityLevel);
        logexp(VerbosityLevel::VERBOSE, [&]() { return preach.print_to_dot(); }, verbosityLevel);

        // check whether safety can be proven, i.e. whether abstract safety holds
        log(VerbosityLevel::NORMAL, "trying to prove abstract safety...", verbosityLevel);
        std::vector<mata::nfa::Nfa> properties_to_check{};
        if (property.has_value()) {
            for (int i{ 0 }; i < dpr.propertyNames.size(); ++i) {
                if (dpr.propertyNames[i] == property.value()) {
                    properties_to_check.push_back(dpr.properties[i]);
                    propertyNames.push_back(dpr.propertyNames[i]);
                }
            }
        } else {
            properties_to_check = dpr.properties;
            propertyNames = dpr.propertyNames;
        }
        std::vector<bool> result = check_abstract_safety(dpr.initialConfig, preach, properties_to_check);
        log(VerbosityLevel::NORMAL, "...done.", verbosityLevel);
        log(VerbosityLevel::QUIET, "Result: " + vec_to_string(result), verbosityLevel);
        logexp(VerbosityLevel::QUIET, [&]() {
            std::ostringstream oss;
            std::vector<std::string> proven {};
            std::vector<std::string> unproven {};
            for (int i{ 0 }; i < result.size(); ++i) {
                if (result[i]) {
                    proven.push_back(propertyNames[i]);
                } else {
                    unproven.push_back(propertyNames[i]);
                }
            }
            oss << "This means that the following properties could be separated from the initial configurations:\n";
            oss << "\t" << vec_to_string(proven) << "\n";
            oss << "While the following properties could not be separated from the initial configurations:\n";
            oss << "\t" << vec_to_string(unproven) << "\n";
            return oss.str();
        }, verbosityLevel, false, false);
    }

    return 0;
}
