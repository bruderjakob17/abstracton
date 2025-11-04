#ifndef ABSTRACTON_UTILS_HPP_
#define ABSTRACTON_UTILS_HPP_

#include <cstddef>
#include <iterator>
#include <vector>

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

#endif
