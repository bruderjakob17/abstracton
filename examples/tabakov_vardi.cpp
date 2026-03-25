#include <mata/nfa/builder.hh>

int main() {
    for (int i = 0; i < 100; ++i) {
        mata::nfa::Nfa nfa = mata::nfa::builder::create_random_nfa_tabakov_vardi(20, 5, 0.5, 0.1);
        if (!nfa.initial.contains(0))
            std::cout << "0 not initial" << std::endl;
    }
}
//
