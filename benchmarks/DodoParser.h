#pragma once

#include <cassert>
#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <stdexcept>
#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nfa/delta.hh>

#include <abstracton/utils/utils.hpp>
#include <abstracton/bddlike/bddlike_nft.hpp>

// struct alphabet_encoding {
//     std::vector<char> alphabet;
//     std::vector<std::string> string_alphabet;
//     std::unordered_map<char, std::string> decoding;
//     std::unordered_map<std::string, char> encoding;
// };

void dfs_explore(std::vector<std::vector<int>> const& adjacency_list, std::vector<int> &order, std::vector<bool> &visited, int node);

std::vector<int> topo_sort(std::vector<std::vector<int>> const& adjacency_list);

// mata::nfa::Nfa parseDodoNfa(Json::Value dfa, alphabet_encoding alphabet_enc);
mata::nfa::Nfa parseDodoNfa(Json::Value nfa, mata::OnTheFlyAlphabet* string_alphabet, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL);

std::pair<std::string, std::string> parsePair(std::string p, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL);

mata::nft::Nft parseTransducer(Json::Value t, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL);

// alphabet_encoding alphabetToCharAlphabet(std::vector<std::string> string_alphabet);

struct DodoParserResult {
    // alphabet_encoding char_alphabet_triple;
    mata::OnTheFlyAlphabet* string_alphabet;
    mata::nfa::Nfa initialConfig;
    std::vector<mata::nfa::Nfa> properties;
    std::vector<std::string> propertyNames;
    mata::nft::Nft transitionRelation;
};

DodoParserResult parseDodoJSON(std::string filepath, int verbosityLevel = logging::DEFAULT_VERBOSITY_LEVEL, bool no_dot_printing = false);

struct BDDlikeDodoParserResult {
    mata::ext::bddlike::SimpleVecAlphabet string_alphabet;
    mata::ext::bddlike::BDDlikeNft initialConfig;
    std::vector<mata::ext::bddlike::BDDlikeNft> properties;
    std::vector<std::string> propertyNames;
    mata::ext::bddlike::BDDlikeNft transitionRelation;
};

/**
 * WARNING: dpr.string_alphabet gets converted to a shared_ptr, taking ownership of *dpr.string_alphabet.
 * Do not use raw pointer dpr.string_alphabet after calling this function!
 */
BDDlikeDodoParserResult convertToBDDlike(DodoParserResult& dpr);
