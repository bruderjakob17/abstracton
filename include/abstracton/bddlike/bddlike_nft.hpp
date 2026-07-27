#pragma once

#include <mata/alphabet.hh>
#include <mata/nfa/nfa.hh>
#include <mata/nft/nft.hh>
#include <mata/nft/delta.hh>
#include <abstracton/utils/utils.hpp>
#include <mata/nft/types.hh>
#include <format>

namespace mata::ext::bddlike {

class VecAlphabetPrinter {
public:
    size_t dimension; // number of tapes this alphabet needs
    VecAlphabetPrinter(size_t dimension) : dimension(dimension) {};
    virtual std::string print(const std::vector<Symbol>& bits) = 0;
    virtual ~VecAlphabetPrinter() = default;

    /**
     * Creates an Nft that accepts all words w that represent a "big symbol" of the alphabet.
     * In particular, all accepted words have a dimension equal to the value stored in the dimension member.
     */
    virtual mata::nft::Nft alphabet_nft() = 0;
};

/**
 * T is the type used to represent a "Big Symbol", that then gets decomposed into a vec of "small symbols".
 * E.g. T might be an integer (Big Symbol), that gets decomposed into its binary digits (small symbols).
 *
 * T can be thought of as "the type through which I want to interact with the automaton".
 *
 */
template<typename T>
class VecAlphabet : public VecAlphabetPrinter {
public:
    VecAlphabet(size_t dimension) : VecAlphabetPrinter(dimension) {}

    virtual std::vector<Symbol> translate_symbol(const T& symbol) = 0;
    // virtual std::vector<Symbol> translate_word(const std::vector<T>& word);

    virtual T reverse_translate_symbol(const std::vector<Symbol>& symbol) = 0;

    std::vector<Symbol> operator[](const T& symbol) { return this->translate_symbol(symbol); }

    std::vector<std::vector<Symbol>> translate_word(const std::vector<T>& word) {
        std::vector<std::vector<Symbol>> result{};
        for (const T& symbol : word) {
            result.push_back(translate_symbol(symbol));
        }
        return result;
    }

    std::vector<T> reverse_translate_word(const std::vector<std::vector<Symbol>>& word) {
        std::vector<T> result{};
        for (const std::vector<Symbol>& symbol : word) {
            result.push_back(reverse_translate_symbol(symbol));
        }
        return result;
    }

    virtual ~VecAlphabet() = default;

protected:
    /**
     * assuming a constant tape size, constructs the alphabet_nfa for this alphabet.
     */
    mata::nft::Nft alphabet_nft(size_t tape_alphabet_size) const {
        using namespace mata::nft;

        Nft aut = Nft::with_levels(dimension, dimension + 1, {0}, {dimension});

        for (State source{ 0 }; source < dimension; ++source) {
            aut.levels[source] = source;
            for (Symbol x{ 0 }; x < tape_alphabet_size; ++x) {
                aut.delta.add(source, x, source + 1);
            }
        }

        aut.levels[dimension] = 0;

        return aut;
    }

    /**
     * assuming a variable tape size, constructs the alphabet_nfa for this alphabet.
     */
    mata::nft::Nft alphabet_nft(const std::vector<size_t>& tape_alphabet_sizes) const {
        using namespace mata::nft;

        Nft aut = Nft::with_levels(dimension, dimension + 1, {0}, {dimension});

        for (State source{ 0 }; source < dimension; ++source) {
            aut.levels[source] = source;
            for (Symbol x{ 0 }; x < tape_alphabet_sizes[source]; ++x) {
                aut.delta.add(source, x, source + 1);
            }
        }

        aut.levels[dimension] = 0;

        return aut;
    }
};

/**
 * SimpleAlphabet is a VecAlphabet where Big Symbol = small symbol, i.e. symbols do not get decomposed.
 *
 * Because of the general framework, one still needs to wrap the small symbol into a vec, e.g. 5 gets represented as [5].
 *
 * Assume the base_alphabet encodes "ef" as 4 and "bab" as 5. Then the functions map as follows:
 * - translate_symbol: "ef" -> [4]
 * - reverse_translate_symbol: [4] -> "ef"
 * - print: [4] -> "ef"
 */
class SimpleVecAlphabet : public VecAlphabet<std::string> {
private:
    std::shared_ptr<Alphabet> base_alphabet;
public:
    using VecAlphabet::alphabet_nft;

    explicit SimpleVecAlphabet(std::shared_ptr<Alphabet> base_alphabet) : VecAlphabet(1), base_alphabet(std::move(base_alphabet)) {}

    std::vector<Symbol> translate_symbol(const std::string& symbol) override {
        return {base_alphabet->translate_symb(symbol)};
    }
    std::string reverse_translate_symbol(const std::vector<Symbol>& symbol) override {
        assert(symbol.size() == 1);
        return base_alphabet->reverse_translate_symbol(symbol[0]);
    }
    std::string print(const std::vector<Symbol>& bits) override {
        assert(bits.size() == 1);
        return base_alphabet->reverse_translate_symbol(bits[0]);
    }

    mata::nft::Nft alphabet_nft() override {
        return alphabet_nft(base_alphabet->get_alphabet_symbols().size());
    }
};

/**
 * In DefaultVecAlphabet(dimension), a big symbol is just a vector of the small symbols it represents,
 * e.g. [0, 15, 1] represents the sequence [0, 15, 1].
 *
 * The functions map as follows:
 * - translate_symbol: [0, 15, 1] -> [0, 15, 1]
 * - reverse_translate_symbol: [0, 15, 1] -> [0, 15, 1]
 * - print: [0, 15, 1] -> "[0, 15, 1]"
 */
class DefaultVecAlphabet : public VecAlphabet<std::vector<Symbol>> {
public:
    using VecAlphabet::alphabet_nft;

    explicit DefaultVecAlphabet(size_t dimension) : VecAlphabet(dimension) {}

    std::vector<Symbol> translate_symbol(const std::vector<Symbol>& symbol) override {
        return symbol;
    }
    std::vector<Symbol> reverse_translate_symbol(const std::vector<Symbol>& symbol) override {
        return symbol;
    }
    std::string print(const std::vector<Symbol>& bits) override {
        return vec_to_string(bits);
    }

    mata::nft::Nft alphabet_nft() override {
        // be careful! introduces DONT CARE symbols which might lead to problems!
        using namespace mata::nft;

        Nft aut = Nft::with_levels(dimension, dimension + 1, {0}, {dimension});

        for (State source{ 0 }; source < dimension; ++source) {
            aut.levels[source] = source;
            aut.delta.add(source, mata::nft::DONT_CARE, source + 1);
        }

        aut.levels[dimension] = 0;

        return aut;
    }
};

/**
 * A PowersetVecAlphabet uses the characteristic sequence to represent a set.
 *
 * Assume the base_alphabet is ["ab", "c", "d"]. Then the functions map as follows:
 * - translate_symbol: ["ab", "c"] -> [1, 1, 0]
 * - reverse_translate_symbol: [1, 1, 0] -> ["ab", "c"]
 * - print: [1, 1, 0] -> "[ab, c]"
 */
class PowersetVecAlphabet : public VecAlphabet<std::vector<std::string>> {
private:
    std::shared_ptr<Alphabet> base_alphabet;
public:
    using VecAlphabet::alphabet_nft;

    explicit PowersetVecAlphabet(std::shared_ptr<Alphabet> base_alphabet) : VecAlphabet(base_alphabet->get_alphabet_symbols().size()), base_alphabet(std::move(base_alphabet)) {
    }

    std::vector<std::string> reverse_translate_symbol(const std::vector<Symbol>& symbol) override {
        std::vector<std::string> result{};
        for (int i{ 0 }; i < dimension; ++i) {
            if (symbol[i] == 1) {
                result.push_back(base_alphabet->reverse_translate_symbol(i));
            }
        }
        return result;
    }
    std::vector<Symbol> translate_symbol(const std::vector<std::string>& set) override {
        std::vector<Symbol> result(dimension, 0);
        for (const std::string& s : set) {
            result[base_alphabet->translate_symb(s)] = 1;
        }
        return result;
    }
    std::string print(const std::vector<Symbol>& bits) override {
        return vec_to_string(reverse_translate_symbol(bits), ", ", "{", "}");
    }

    mata::nft::Nft alphabet_nft() override {
        return alphabet_nft(2);
    }
};

/**
 * Use fixed-length base encoding (most significant digit first).
 *
 * Assume base = 2, length = 4. Then the functions map as follows:
 * - translate_symbol: 5 -> [0, 1, 0, 1]
 * - reverse_translate_symbol: [0, 1, 0, 1] -> 5
 * - print: [0, 1, 0, 1] -> "5"
 */
class BaseMSDVecAlphabet : public VecAlphabet<long> {
private:
    size_t base;
public:
    using VecAlphabet::alphabet_nft;

    explicit BaseMSDVecAlphabet(size_t base, size_t length) : VecAlphabet(length), base(base) {}

    std::vector<Symbol> translate_symbol(const long& symbol) override {
        // map e.g. 5 -> [0, 1, 0, 1] if base = 2 and length = 4
        std::vector<Symbol> digits(dimension, 0);
        long number = symbol;

        int i = dimension - 1;
        while (number > 0) {
            if (i < 0) {
                throw std::runtime_error(std::format("error: cannot represent {} in base {} using {} digits", symbol, base, dimension));
            }
            digits[i] = number % base;
            number /= base;
            --i;
        }

        return digits;
    }
    long reverse_translate_symbol(const std::vector<Symbol>& bits) override {
        // map e.g. [0, 1, 0, 1] -> 5 if base = 2 and length = 4
        long result{0};

        for (int i{0}; i < bits.size(); ++i) {
            result = base * result + bits[i];
        }

        return result;
    }
    std::string print(const std::vector<Symbol>& bits) override {
        return std::to_string(reverse_translate_symbol(bits));
    }

    mata::nft::Nft alphabet_nft() override {
        return alphabet_nft(base);
    }
};

/**
 * Use fixed-length base encoding (most significant digit first).
 *
 * Assume base = 2, length = 4. Then the functions map as follows:
 * - translate_symbol: 5 -> [1, 0, 1, 0]
 * - reverse_translate_symbol: [1, 0, 1, 0] -> 5
 * - print: [1, 0, 1, 0] -> "5"
 */
class BaseLSDVecAlphabet : public VecAlphabet<long> {
private:
    size_t base;
public:
    using VecAlphabet::alphabet_nft;

    explicit BaseLSDVecAlphabet(size_t base, size_t length) : VecAlphabet(length), base(base) {}

    std::vector<Symbol> translate_symbol(const long& symbol) override {
        // map e.g. 5 -> [1, 0, 1, 0] if base = 2 and length = 4
        std::vector<Symbol> digits(dimension, 0);
        long number = symbol;

        int i = 0;
        while (number > 0) {
            if (i >= dimension) {
                throw std::runtime_error(std::format("error: cannot represent {} in base {} using {} digits", symbol, base, dimension));
            }
            digits[i] = number % base;
            number /= base;
            ++i;
        }

        return digits;
    }
    long reverse_translate_symbol(const std::vector<Symbol>& bits) override {
        // map e.g. [1, 0, 1, 0] -> 5 if base = 2 and length = 4
        long result{0};

        for (int i{static_cast<int>(bits.size()) - 1}; i >= 0; --i) {
            result = base * result + bits[i];
        }

        return result;
    }
    std::string print(const std::vector<Symbol>& bits) override {
        return std::to_string(reverse_translate_symbol(bits));
    }

    mata::nft::Nft alphabet_nft() override {
        return alphabet_nft(base);
    }
};

class BDDlikeNft : public nft::Nft {
private:
    using super = nft::Nft;
    using Nft::is_in_lang_by_levels;
public:
    std::vector<size_t> alphabet_sizes; // TODO rename to alphabet_dimensions, TODO add number of alphabets as maintained member
    std::vector<size_t> high_levels; // maps an internal level to the correct "high level".
    std::vector<size_t> alphabet_sizes_prefixsums;
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

    explicit BDDlikeNft(
        mata::nft::Nft aut
    ) : Nft{ std::move(aut) } {}

    explicit BDDlikeNft(
        mata::nft::Nft aut,
        std::vector<size_t> alphabet_sizes,
        std::vector<std::shared_ptr<VecAlphabetPrinter>> alphabets
    ) : Nft{ std::move(aut) }, alphabet_sizes{std::move(alphabet_sizes)}, alphabets{std::move(alphabets)} {}

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
            assert(result.alphabet_sizes.size() == alphabets.value().size()); // TODO also assert that each alphabet has the correct number of tapes
            result.alphabets = alphabets.value();
        } else {
            result.alphabets = std::vector<std::shared_ptr<VecAlphabetPrinter>>{};
            for (int i{ 0 }; i < alphabet_sizes.size(); ++i) {
                result.alphabets.push_back(std::make_shared<DefaultVecAlphabet>(DefaultVecAlphabet{alphabet_sizes[i]}));
            }
        }

        result.levels = mata::nft::Levels{ total_number_of_tapes, num_of_states, mata::nft::DEFAULT_LEVEL };

        return result;
    }

    /**
     * Creates a new BDDlikeNft with one tape of dimension 1.
     * The created alphabet is a SimpleVecAlphabet on the given alphabet (and can be cast back to it).
     */
    static BDDlikeNft from_nfa_with_alphabet(mata::nfa::Nfa aut, std::shared_ptr<Alphabet> alphabet) {
        SimpleVecAlphabet alph(alphabet);
        std::shared_ptr<VecAlphabetPrinter> alph_ptr{std::make_shared<SimpleVecAlphabet>(alph)};
        return BDDlikeNft{Nft{aut}, std::vector<size_t>{1}, std::vector<std::shared_ptr<VecAlphabetPrinter>>{alph_ptr}};
    }

    /**
     * Prints the automaton at *high-level* only, meaning it does *not* show its internal structure, but e.g. prints
     * s -(a, [a, b])-> t
     * if the automaton has as SimpleVecAlphabet containing a at the first tape and a PowersetVecAlphabet containing
     * [a, b] at the second "tape".
     *
     * To see the internal structure, use print_to_dot().
     */
    void print_to_dot_using_alphabets(std::ostream &output) const;
    std::string print_to_dot_using_alphabets() const;
    /**
     * A "Word" is now a vector of symbol vectors, as each symbol vector represents one letter.
     */
    bool is_in_lang_by_levels(const std::vector<std::vector<std::vector<Symbol>>> level_words, bool match_prefix = false) const;

    Nft to_nft_copy() const { return Nft{ delta, initial, final, levels }; };
    Nft to_nft_move() { return Nft{ std::move(delta), std::move(initial), std::move(final), std::move(levels) }; }

    /**
     * Initializes high_levels and alphabet_sizes_prefixsums vector, if uninitialized.
     */
    void initialize_maintained_metadata() {
        if (high_levels.empty()) {
            high_levels = std::vector<size_t>(levels.num_of_levels, 0);
            alphabet_sizes_prefixsums = std::vector<size_t>(alphabet_sizes.size(), 0);
            int levels_seen = 0;
            for (int i = 0; i < alphabet_sizes.size(); ++i) {
                for (int j = 0; j < alphabet_sizes[i]; ++j) {
                    high_levels[levels_seen + j] = alphabet_sizes[i];
                }
                alphabet_sizes_prefixsums[i] = levels_seen;
                levels_seen += alphabet_sizes[i];
            }
        }
    }
    size_t get_high_level(size_t internal_level) {
        initialize_maintained_metadata();
        assert(internal_level < levels.num_of_levels);
        return high_levels[internal_level];
    }

    std::vector<size_t> get_internal_levels(mata::nft::Level high_level) {
        initialize_maintained_metadata();
        assert(high_level < alphabet_sizes.size());
        std::vector<size_t> result(alphabet_sizes[high_level], 0);
        for (int i = 0; i < alphabet_sizes[high_level]; ++i) {
            result[i] = alphabet_sizes_prefixsums[high_level] + i;
        }
        return result;
    }

    mata::utils::OrdVector<mata::nft::Level> get_internal_levels(const mata::utils::OrdVector<mata::nft::Level>& high_levels) {
        utils::OrdVector<mata::nft::Level> internal_levels{};
        for (mata::nft::Level high_lvl : high_levels) {
            // append internal levels of high level
            for (size_t internal_lvl : get_internal_levels(high_lvl)) {
                internal_levels.insert(internal_lvl);
            }
        }
        return internal_levels;
    }
};

BDDlikeNft minimize(const BDDlikeNft& aut);
BDDlikeNft compose(BDDlikeNft& lhs, BDDlikeNft& rhs,
            const utils::OrdVector<mata::nft::Level>& lhs_sync_high_levels, const utils::OrdVector<mata::nft::Level>& rhs_sync_high_levels,
            bool project_out_sync_levels = true,
            mata::nft::JumpMode jump_mode = mata::nft::JumpMode::RepeatSymbol);
BDDlikeNft project_to(BDDlikeNft& nft, const mata::utils::OrdVector<mata::nft::Level>& high_levels_to_project, mata::nft::JumpMode jump_mode = mata::nft::JumpMode::RepeatSymbol);
inline BDDlikeNft project_to(BDDlikeNft& nft, mata::nft::Level high_level_to_project, mata::nft::JumpMode jump_mode = mata::nft::JumpMode::RepeatSymbol) {
    return mata::ext::bddlike::project_to(nft, mata::utils::OrdVector<mata::nft::Level>{high_level_to_project}, jump_mode);
}

BDDlikeNft create_sigma_star_nft(int number_of_levels, const VecAlphabetPrinter& alphabet);
BDDlikeNft create_sigma_star_nft(const std::vector<VecAlphabetPrinter>& alphabets);

}
