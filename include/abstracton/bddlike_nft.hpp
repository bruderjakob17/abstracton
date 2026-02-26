#pragma once

#include <mata/alphabet.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/delta.hh>
#include <abstracton/utils/utils.hpp>
#include <mata/nft/types.hh>

namespace mata::ext {

struct VecAlphabetPrinter {
    virtual std::string print(const std::vector<Symbol>& bits) = 0;
    virtual ~VecAlphabetPrinter() = default;
};

/**
 * T is the type used to represent a "Big Symbol", that then gets decomposed into a vec of "small symbols".
 * E.g. T might be an integer (Big Symbol), that gets decombosed into its binary digits (small symbols).
 */
template<typename T>
class VecAlphabet : public VecAlphabetPrinter {
public:
    virtual std::vector<Symbol> translate_symbol(const T& symbol) = 0;
    // virtual std::vector<Symbol> translate_word(const std::vector<T>& word);

    virtual T reverse_translate_symbol(std::vector<Symbol> symbol) = 0;

    std::vector<Symbol> operator[](const T& symbol) { return this->translate_symbol(symbol); }

    virtual ~VecAlphabet() = default;
};

class DefaultVecAlphabet : public VecAlphabet<std::vector<Symbol>> {
public:
    std::vector<Symbol> translate_symbol(const std::vector<Symbol>& symbol) override {
        return symbol;
    }
    std::vector<Symbol> reverse_translate_symbol(std::vector<Symbol> symbol) override {
        return symbol;
    }
    std::string print(const std::vector<Symbol>& bits) override {
        return vec_to_string(bits);
    }
};

class SupersetVecAlphabet : public VecAlphabet<std::vector<std::string>> {
private:
    Alphabet* base_alphabet;
    size_t base_alphabet_size;
public:
    explicit SupersetVecAlphabet(Alphabet* base_alphabet) : base_alphabet(std::move(base_alphabet)) {
        base_alphabet_size = base_alphabet->get_alphabet_symbols().size();
    }

    std::vector<std::string> reverse_translate_symbol(std::vector<Symbol> symbol) override {
        std::vector<std::string> result{};
        for (int i{ 0 }; i < base_alphabet_size; ++i) {
            if (symbol[i] == 1) {
                result.push_back(base_alphabet->reverse_translate_symbol(i));
            }
        }
        return result;
    }
    std::vector<Symbol> translate_symbol(const std::vector<std::string>& set) override {
        std::vector<Symbol> result(base_alphabet_size, 0);
        for (const std::string& s : set) {
            result[base_alphabet->translate_symb(s)] = 1;
        }
        return result;
    }
    std::string print(const std::vector<Symbol>& bits) override {
        return vec_to_string(reverse_translate_symbol(bits));
    }
};

class BDDlikeNft : public nft::Nft {
private:
    using super = nft::Nft;
public:
    std::vector<size_t> alphabet_sizes;
    std::vector<std::shared_ptr<VecAlphabetPrinter>> alphabets;

    explicit BDDlikeNft(
        mata::nft::Delta delta = {},
        utils::SparseSet<mata::nft::State> initial_states = {},
        utils::SparseSet<mata::nft::State> final_states = {},
        mata::nft::Levels levels = {}
    ) : nft::Nft{ std::move(delta), std::move(initial_states), std::move(final_states), std::move(levels)} {}

    explicit BDDlikeNft(
        const size_t num_of_states,
        utils::SparseSet<mata::nft::State> initial_states = {},
        utils::SparseSet<mata::nft::State> final_states = {},
        mata::nft::Levels levels = {}
    ) : Nft{ num_of_states, std::move(initial_states), std::move(final_states), std::move(levels)} {}

    static BDDlikeNft with_alphabet_sizes(
        std::vector<size_t> alphabet_sizes,
        const size_t num_of_states = 0,
        utils::SparseSet<mata::nft::State> initial_states = {},
        utils::SparseSet<mata::nft::State> final_states = {},
        std::optional<std::vector<std::shared_ptr<VecAlphabetPrinter>>> alphabets = std::nullopt
    ) {
        size_t total_number_of_tapes = 0;
        for (int i{ 0 }; i < alphabet_sizes.size(); ++i) {
            total_number_of_tapes += alphabet_sizes[i];
        }

        BDDlikeNft result{num_of_states, std::move(initial_states), std::move(final_states)};
        result.alphabet_sizes = alphabet_sizes;

        if (alphabets.has_value()) {
            result.alphabets = alphabets.value();
        } else {
            result.alphabets = std::vector<std::shared_ptr<VecAlphabetPrinter>>(alphabet_sizes.size(), std::make_shared<DefaultVecAlphabet>(DefaultVecAlphabet{}));
        }

        result.levels = mata::nft::Levels{ total_number_of_tapes, num_of_states, mata::nft::DEFAULT_LEVEL };

        return result;
    }
};

}
