#include <cassert>
#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <stdexcept>
#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nfa/delta.hh>

// struct alphabet_encoding {
//     std::vector<char> alphabet;
//     std::vector<std::string> string_alphabet;
//     std::unordered_map<char, std::string> decoding;
//     std::unordered_map<std::string, char> encoding;
// };

void dfs_explore(std::vector<std::vector<int>> const& adjacency_list, std::vector<int> &order, std::vector<bool> &visited, int node);

std::vector<int> topo_sort(std::vector<std::vector<int>> const& adjacency_list);

// mata::nfa::Nfa parseDodoNfa(Json::Value dfa, alphabet_encoding alphabet_enc);
mata::nfa::Nfa parseDodoNfa(Json::Value nfa, mata::OnTheFlyAlphabet* string_alphabet);

std::pair<std::string, std::string> parsePair(std::string p);

mata::nft::Nft parseTransducer(Json::Value t);

// alphabet_encoding alphabetToCharAlphabet(std::vector<std::string> string_alphabet);

struct DodoParserResult {
    // alphabet_encoding char_alphabet_triple;
    mata::OnTheFlyAlphabet string_alphabet;
    mata::nfa::Nfa initialConfig;
    std::vector<mata::nfa::Nfa> properties;
    mata::nft::Nft transitionRelation;
};

DodoParserResult parseDodoJSON(std::string filepath);
