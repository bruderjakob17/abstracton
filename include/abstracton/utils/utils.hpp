#ifndef ABSTRACTON_UTILS_HPP_
#define ABSTRACTON_UTILS_HPP_

#include <cstddef>
#include <iterator>
#include <vector>
#include <random> // for PRNG based hashing using mersenne twister

template<>
struct std::hash<std::unordered_set<int>> {
    std::size_t operator()(const std::unordered_set<int>& s) const noexcept {
        std::mt19937 mt {s.size()};
        int a = mt();
        int b = mt();
        std::size_t result { 0 };
        for (int i : s) {
            result ^= a * i + b * i;
        }
        return result;
    }
};

template<typename T>
std::vector<T> set_to_vec(std::unordered_set<T> set) {
    std::vector<T> result {};
    result.insert(result.end(), set.begin(), set.end());
    return result;
}

// TODO generalize int to template type T
class BoundedTuples {
public:
    using tuple_type = std::vector<unsigned>;

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type        = tuple_type;
        using difference_type   = std::ptrdiff_t;
        using pointer           = tuple_type*;
        using reference         = tuple_type&;

        Iterator() : limits_ptr(nullptr), done(true) {}

        Iterator(const std::vector<unsigned>* limits)
            : limits_ptr(limits),
              current(limits->size(), 0),
              done(limits->empty())
        {
            if (std::any_of(limits->cbegin(), limits->cend(), [](unsigned i) { return i == 0; })) {
                done = true;
            }
        }

        reference operator*() {
            return current;
        }

        Iterator& operator++() {
            // increment as far to the back as possible (we want to iterate lexicographically)
            int i = (int)limits_ptr->size() - 1;
            while (i >= 0) {
                if (current[i] + 1 < (*limits_ptr)[i]) {
                    current[i]++;
                    // reset entries after the incremented entry to 0
                    for (int j = i + 1; j < (int)limits_ptr->size(); j++)
                        current[j] = 0;
                    return *this;
                }
                i--;
            }
            done = true;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const {
            if (done && other.done) return true;
            if (done != other.done) return false;
            return (limits_ptr == other.limits_ptr)
                   && (current == other.current);
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }

    private:
        const std::vector<unsigned>* limits_ptr;
        tuple_type current;
        bool done = false;
    };

    explicit BoundedTuples(std::vector<unsigned> limits)
        : limits(std::move(limits))
    {}

    Iterator begin() const {
        return Iterator(&limits);
    }
    Iterator end() const {
        return Iterator();  // done = true
    }

private:
    std::vector<unsigned> limits;
};

// TODO generalize int to template type T
template<typename T>
class Subsets {
public:
    using subset_type = std::unordered_set<T>;

    struct Iterator {
        using iterator_category = std::input_iterator_tag;
        using value_type        = subset_type;
        using difference_type   = std::ptrdiff_t;
        using pointer           = subset_type*;
        using reference         = subset_type;

        Iterator() : elements_ptr(nullptr), done(true) {}

        Iterator(const std::vector<T>* elements)
            : elements_ptr(elements),
              current(elements->size(), false),
              done(false)
        {}

        reference operator*() {
            std::unordered_set<T> result {};
            for (int i = 0; i < current.size(); ++i) {
                if (current[i]) {
                    result.insert(elements_ptr->operator[](i));
                }
            }
            return result;
        }

        Iterator& operator++() {
            // binary increment of bool array (left to right)
            for (int i = 0; i < current.size(); ++i) {
                if (current[i]) {
                    current[i] = false;
                } else {
                    current[i] = true;
                    return *this;
                }
            }
            done = true;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const {
            if (done && other.done) return true;
            if (done != other.done) return false;
            return (elements_ptr == other.elements_ptr)
                   && (current == other.current);
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }

    private:
        const std::vector<T>* elements_ptr;
        std::vector<bool> current;
        bool done = false;
    };

    explicit Subsets(std::unordered_set<T> elements)
        : elements(set_to_vec(elements))
    {}

    Iterator begin() const {
        return Iterator(&elements);
    }
    Iterator end() const {
        return Iterator();  // done = true
    }

private:
    std::vector<T> elements;
};

#endif
