#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>
#include <abstracton/mata_extensions.hpp>
#include <abstracton/abstracton.hpp>
#include <abstracton/utils/utils.hpp>
#include <abstracton/bddlike/bddlike_nft.hpp>
#include <abstracton/bddlike/interpretations.hpp>
#include <abstracton/bddlike/abstracton.hpp>
#include <format>
#include <queue>

#define INIT_CLOCKS() std::chrono::steady_clock::time_point begin, end;
#define TICK() if (measure_time) { begin = std::chrono::steady_clock::now(); }
#define TOCK(message) if (measure_time) { end = std::chrono::steady_clock::now(); std::cout << "Time needed for " << message << ": " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl; }

#define PRINT_AUT(name, aut) \
logging::log(logging::VerbosityLevel::VERBOSE, std::format("automaton {}:", name), verbosityLevel); \
if (no_dot_printing) { \
    logging::logexp(logging::VerbosityLevel::VERBOSE, [&]() { return std::format("{} states", aut.num_of_states()); }, verbosityLevel); \
} else { \
    logging::logexp(logging::VerbosityLevel::VERBOSE, [&]() { return aut.print_to_dot(); }, verbosityLevel); \
}

using namespace mata;
using namespace mata::ext::bddlike;

/// uses the fact that ind = pi_2((inv(delta) V) cap V_comp)
mata::ext::bddlike::BDDlikeNft compute_ind_new(mata::ext::bddlike::BDDlikeNft& abstraction_framework, mata::ext::bddlike::BDDlikeNft& transition_relation, bool exclude_empty_abstractions, int verbosityLevel, bool measure_time, bool no_dot_printing) {
    INIT_CLOCKS();

    using namespace std;
    using namespace mata::utils;

    TICK();
    BDDlikeNft delta_v {mata::ext::bddlike::compose(transition_relation, abstraction_framework, {0}, {0})};
    TOCK("computing composition v_delta of abstraction framework and transition relation");
    PRINT_AUT("inv(->) o V", delta_v);

    // TICK();
    // delta_v = mata::nft::reduce(delta_v);
    // TOCK("reducing inv(->) o V");
    // PRINT_AUT("reduce(inv(->) o V)", delta_v);

    TICK();
    BDDlikeNft v_complement {mata::ext::bddlike::complement(abstraction_framework, true)};
    TOCK("computing complement v_complement of abstraction framework");
    PRINT_AUT("complement(V)", v_complement);

    TICK()
    v_complement = mata::ext::bddlike::minimize(v_complement);
    TOCK("minimizing complement(V)");
    PRINT_AUT("min(complement(V))", v_complement);

    TICK();
    BDDlikeNft preprojection {mata::ext::bddlike::intersection(delta_v, v_complement)};
    TOCK("computing intersection 'preprojection' of delta_v and v_complement");
    PRINT_AUT("preprojection", preprojection);

    TICK();
    BDDlikeNft projection{ mata::ext::bddlike::project_to(preprojection, 1) };
    TOCK("computing complement of ind (i.e. projecting preprojection onto second tape)");
    PRINT_AUT("complement(ind)", projection);

    TICK();
    projection = mata::ext::bddlike::minimize(projection);
    TOCK("minimizing projection");
    PRINT_AUT("min(projection)", projection);

    TICK();
    BDDlikeNft ind{ mata::ext::bddlike::complement(projection) };
    TOCK("computing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind has {} states", ind.num_of_states()), verbosityLevel);
    TICK();
    ind = mata::ext::bddlike::minimize(ind);
    TOCK("minimizing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind has {} states", ind.num_of_states()), verbosityLevel);
    if (exclude_empty_abstractions) {
        // intersect with pi_1(V)
        logging::log(logging::VerbosityLevel::DEBUGV, "ind with empty abstractions:", verbosityLevel);
        logging::logexp(logging::VerbosityLevel::DEBUGV, [&]() { return ind.print_to_dot(); }, verbosityLevel);
        TICK();
        auto result = mata::ext::bddlike::intersection(ind, mata::ext::bddlike::project_to(abstraction_framework, 1));
        TOCK("excluding empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        TICK();
        result = mata::ext::bddlike::minimize(result);
        TOCK("minimizing ind without empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        return result;
    } else {
        return ind;
    }


    // comment below: if abstraction framework has alphabets (abstract, concrete)

    /// uses the fact that pi(Id cap (V delta V_comp_inv)) = pi_1((V delta) cap V_comp)

    /*
    INIT_CLOCKS();

    using namespace std;
    using namespace mata::utils;

    TICK();
    BDDlikeNft v_delta {compose(abstraction_framework, transition_relation)};
    TOCK("computing composition v_delta of abstraction framework and transition relation");
    PRINT_AUT("V o ->", v_delta);

    // TICK();
    // v_delta = mata::nft::reduce(v_delta);
    // TOCK("reducing V o ->");
    // PRINT_AUT("reduce(V o ->)", v_delta);

    TICK();
    BDDlikeNft v_complement {mata::ext::bddlike::complement(abstraction_framework, true)};
    TOCK("computing complement v_complement of abstraction framework");
    PRINT_AUT("complement(V)", v_complement);

    TICK()
    v_complement = mata::ext::bddlike::minimize(v_complement);
    TOCK("minimizing complement(V)");
    PRINT_AUT("min(complement(V))", v_complement);

    TICK();
    BDDlikeNft preprojection {mata::ext::bddlike::intersection(v_delta, v_complement)};
    TOCK("computing intersection 'preprojection' of v_delta and v_complement");
    PRINT_AUT("preprojection", preprojection);

    TICK();
    BDDlikeNft projection{ mata::ext::bddlike::project_to(preprojection, 0) };
    TOCK("computing complement of ind (i.e. projecting preprojection onto first tape)");
    PRINT_AUT("complement(ind)", projection);

    TICK();
    projection = mata::ext::bddlike::minimize(projection);
    TOCK("minimizing projection");
    PRINT_AUT("min(projection)", projection);

    TICK();
    BDDlikeNft ind{ mata::ext::bddlike::complement(projection) };
    TOCK("computing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind has {} states", ind.num_of_states()), verbosityLevel);
    TICK();
    ind = mata::ext::bddlike::minimize(ind);
    TOCK("minimizing ind");
    logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind has {} states", ind.num_of_states()), verbosityLevel);
    if (exclude_empty_abstractions) {
        // intersect with pi_1(V)
        logging::log(logging::VerbosityLevel::DEBUGV, "ind with empty abstractions:", verbosityLevel);
        logging::logexp(logging::VerbosityLevel::DEBUGV, [&]() { return ind.print_to_dot(); }, verbosityLevel);
        TICK();
        auto result = mata::ext::bddlike::intersection(ind, mata::ext::bddlike::project_to(abstraction_framework, 0));
        TOCK("excluding empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        TICK();
        result = mata::ext::bddlike::minimize(result);
        TOCK("minimizing ind without empty abstractions");
        logging::log(logging::VerbosityLevel::VERBOSE, std::format("minimized ind without empty abstractions has {} states", result.num_of_states()), verbosityLevel);
        return result;
    } else {
        return ind;
    }
    */
}
