#include <list>
#include <vector>
#include <cmath>
#include <stdexcept>

// Robin Hood hashing

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
private:
    using Pair = std::pair<const KeyType, ValueType>;
    using PairIterator = typename std::list<Pair>::iterator;
    struct array_element {
        PairIterator iter;
        int32_t distance;
    };

    Hash hasher_;
    size_t size_;
    std::list<Pair> elements_list_;
    std::vector<array_element> array_;
    double LOAD_FACTOR = 0.65;

public:
    using iterator = typename std::list<std::pair<const KeyType, ValueType>>::iterator;
    using const_iterator = typename std::list<std::pair<const KeyType, ValueType>>::const_iterator;

    HashMap(Hash new_hasher = Hash()) : hasher_(new_hasher) {
        size_ = 0;
    }

    HashMap(const HashMap &other) {
        size_ = 0;
        LOAD_FACTOR = other.LOAD_FACTOR;
        elements_list_.clear();
        array_.clear();
        for (const auto& p : other.elements_list_) {
            insert(p);
        }
    }

    HashMap& operator=(const HashMap &other) {
        size_ = 0;
        LOAD_FACTOR = other.LOAD_FACTOR;
        auto temp = other.elements_list_;
        elements_list_.clear();
        array_.clear();
        for (const auto& p : temp) {
            insert(p);
        }
        return *this;
    }

    template<typename Iterator>
    HashMap(Iterator begin, Iterator end, Hash new_hasher = Hash()) : hasher_(new_hasher) {
        size_ = 0;
        for (; begin != end; ++begin) {
            insert({begin->first, begin->second});
        }
    }

    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>> &list, Hash new_hasher = Hash()) {
        size_ = 0;
        for (const auto &p : list) {
            (*this)[p.first] = p.second;
        }
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    Hash hash_function() const {
        return hasher_;
    }

    void clear() {
        array_.clear();
        elements_list_.clear();
        size_ = 0;
    }

    void resize_map() {
        size_t new_size = 2 * array_.size();
        if (double(new_size) < std::ceil(double(size_ + 1) / LOAD_FACTOR)) {
            new_size = std::ceil(double(size_ + 1) / LOAD_FACTOR);
        }
        size_ = 0;
        array_.clear();
        auto temp = elements_list_;
        elements_list_.clear();
        array_.resize(new_size, {.iter = elements_list_.end(), .distance = -1});
        for (const auto& p : temp) {
            insert(p);
        }
    }

    iterator begin() {
        return elements_list_.begin();
    }

    const_iterator begin() const {
        return elements_list_.begin();
    }

    iterator end() {
        return elements_list_.end();
    }

    const_iterator end() const {
        return elements_list_.end();
    }

    void insert(const std::pair<const KeyType, ValueType> &p) {
        if (size_ + 1 >= array_.size() * LOAD_FACTOR) {
            resize_map();
        }
        size_t current_position = hasher_(p.first) % array_.size();
        int32_t current_distance = 0;
        bool is_already_inserted = false;

        while (array_[current_position].distance != -1) {
            auto iter = array_[current_position].iter;
            if (iter->first == p.first) {
                is_already_inserted = true;
                break;
            }
            current_position = (current_position + 1) % array_.size();
        }

        if (!is_already_inserted) {
            ++size_;
            elements_list_.push_back({p.first, p.second});
            current_position = hasher_(p.first) % array_.size();
            auto current_iterator = --elements_list_.end();
            while (array_[current_position].distance != -1) {
                if (array_[current_position].distance < current_distance) {
                    std::swap(array_[current_position].distance, current_distance);
                    std::swap(array_[current_position].iter, current_iterator);
                }
                current_position = (current_position + 1) % array_.size();
                ++current_distance;
            }
            array_[current_position].distance = current_distance;
            array_[current_position].iter = current_iterator;
        }
    }

    void erase(const KeyType &key) {
        int32_t current_position = get_key_position(key);
        if (current_position != -1) {
            --size_;
            elements_list_.erase(array_[current_position].iter);
            int32_t next_position = (current_position + 1) % array_.size();
            while (array_[next_position].distance > 0) {
                array_[current_position].iter = array_[next_position].iter;
                array_[current_position].distance = array_[next_position].distance - 1;
                current_position = next_position;
                next_position = (next_position + 1) % array_.size();
            }
            array_[current_position].distance = -1;
        }
    }

    int32_t get_key_position(const KeyType &key) const {
        if (size_ > 0) {
            int32_t current_position = hasher_(key) % array_.size();
            while (array_[current_position].distance != -1) {
                if (array_[current_position].iter->first == key) {
                    return current_position;
                }
                current_position = (current_position + 1) % array_.size();
            }
        }
        return -1;
    }

    iterator find(const KeyType &key) {
        int32_t pos = get_key_position(key);
        if (pos != -1) {
            return array_[pos].iter;
        }
        return elements_list_.end();
    }

    const_iterator find(const KeyType &key) const {
        int32_t pos = get_key_position(key);
        if (pos != -1) {
            return array_[pos].iter;
        }
        return elements_list_.end();
    }

    const ValueType& at(const KeyType &key) const {
        int32_t pos = get_key_position(key);
        if (pos != -1) {
            return array_[pos].iter->second;
        }
        throw std::out_of_range("Element not found.");
    }

    ValueType& operator[](const KeyType &key) {
        int32_t pos = get_key_position(key);
        if (pos != -1) {
            return array_[pos].iter->second;
        }
        insert({key, ValueType()});
        return operator[](key);
    }
};
