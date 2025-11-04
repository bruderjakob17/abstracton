#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/builder.hh>
#include <abstracton/abstracton.hpp>
#include <abstracton/utils/utils.hpp>
#include <abstracton/mata_extensions.hpp>

using namespace mata;
using namespace mata::nfa;
using namespace mata::nft;

int main() {
    Nft nft = mata::ext::builder::create_random_nft_tabakov_vardi(2, 12, {2, 2}, 1.0, 0.3);
    std::cout << nft.print_to_dot(true) << std::endl;

    return 0;
}
