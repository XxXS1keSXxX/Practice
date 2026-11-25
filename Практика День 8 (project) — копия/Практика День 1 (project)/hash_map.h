
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <utility>
#include <cstddef>
#include <iterator>

template <typename K>
struct HashFunction {
    std::size_t operator()(const K& key) const {
        return std::hash<K>{}(key);
    }
};

template <>
struct HashFunction<std::string> {
    std::size_t operator()(const std::string& key) const {
        std::size_t hash = 0;

        for (char c : key) {
            hash = hash * 31 + static_cast<unsigned char>(c);
        }

        return hash;
    }
};

template <typename K, typename V, typename Hash = HashFunction<K>>
class HashMap {
private:
    struct Node {
        std::pair<const K, V> data;
        std::unique_ptr<Node> next;

        Node(const K& key, const V& value)
            : data(key, value), next(nullptr) {}
    };

    std::vector<std::unique_ptr<Node>> buckets_;
    std::size_t size_ = 0;
    Hash hash_;

    void rehash(std::size_t newCount) {
        if (newCount == 0) {
            throw std::invalid_argument(
                "Кількість кошиків має бути більшою за 0"
            );
        }

        std::vector<std::unique_ptr<Node>> newBuckets(newCount);

        for (auto& bucket : buckets_) {
            while (bucket) {
                auto node = std::move(bucket);
                bucket = std::move(node->next);

                std::size_t index =
                    hash_(node->data.first) % newCount;

                node->next = std::move(newBuckets[index]);
                newBuckets[index] = std::move(node);
            }
        }

        buckets_ = std::move(newBuckets);
    }

public:

    class ConstIterator;

    class Iterator {
    private:
        HashMap* map_;
        std::size_t bucketIndex_;
        Node* node_;

        Iterator(HashMap* map, std::size_t index, Node* node)
            : map_(map), bucketIndex_(index), node_(node) {}

        friend class HashMap;
        friend class ConstIterator;

    public:
        Iterator()
            : map_(nullptr), bucketIndex_(0), node_(nullptr) {}

        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<const K, V>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        reference operator*() const {
            return node_->data;
        }

        pointer operator->() const {
            return &node_->data;
        }

        Iterator& operator++() {
            if (!node_ || !map_) {
                return *this;
            }

            if (node_->next) {
                node_ = node_->next.get();
            }
            else {
                ++bucketIndex_;

                while (bucketIndex_ < map_->buckets_.size() &&
                    !map_->buckets_[bucketIndex_]) {
                    ++bucketIndex_;
                }

                node_ = bucketIndex_ < map_->buckets_.size()
                    ? map_->buckets_[bucketIndex_].get()
                    : nullptr;
            }

            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const Iterator& other) const {
            return map_ == other.map_ && node_ == other.node_;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
    };

    class ConstIterator {
    private:
        const HashMap* map_;
        std::size_t bucketIndex_;
        const Node* node_;

        ConstIterator(
            const HashMap* map,
            std::size_t index,
            const Node* node
        )
            : map_(map), bucketIndex_(index), node_(node) {}

        friend class HashMap;

    public:
        ConstIterator()
            : map_(nullptr), bucketIndex_(0), node_(nullptr) {}

        ConstIterator(const Iterator& other)
            : map_(other.map_),
            bucketIndex_(other.bucketIndex_),
            node_(other.node_) {}

        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<const K, V>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;

        reference operator*() const {
            return node_->data;
        }

        pointer operator->() const {
            return &node_->data;
        }

        ConstIterator& operator++() {
            if (!node_ || !map_) {
                return *this;
            }

            if (node_->next) {
                node_ = node_->next.get();
            }
            else {
                ++bucketIndex_;

                while (bucketIndex_ < map_->buckets_.size() &&
                    !map_->buckets_[bucketIndex_]) {
                    ++bucketIndex_;
                }

                node_ = bucketIndex_ < map_->buckets_.size()
                    ? map_->buckets_[bucketIndex_].get()
                    : nullptr;
            }

            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const ConstIterator& other) const {
            return map_ == other.map_ && node_ == other.node_;
        }

        bool operator!=(const ConstIterator& other) const {
            return !(*this == other);
        }
    };

    using iterator = Iterator;
    using const_iterator = ConstIterator;

    explicit HashMap(std::size_t bucketCount = 8)
        : buckets_(bucketCount) {

        if (bucketCount == 0) {
            throw std::invalid_argument(
                "Кількість кошиків має бути більшою за 0"
            );
        }
    }

    HashMap(const HashMap& other)
        : buckets_(other.buckets_.empty() ? 8 : other.buckets_.size()),
        size_(other.size_),
        hash_(other.hash_) {

        for (std::size_t i = 0; i < other.buckets_.size(); ++i) {
            const Node* current = other.buckets_[i].get();
            auto* tail = &buckets_[i];

            while (current) {
                *tail = std::make_unique<Node>(
                    current->data.first,
                    current->data.second
                );

                tail = &((*tail)->next);
                current = current->next.get();
            }
        }
    }

    HashMap(HashMap&& other)
        : buckets_(std::move(other.buckets_)),
        size_(other.size_),
        hash_(std::move(other.hash_)) {

        other.size_ = 0;
    }

    HashMap& operator=(const HashMap& other) {
        if (this != &other) {
            HashMap temp(other);
            swap(temp);
        }

        return *this;
    }

    HashMap& operator=(HashMap&& other) {
        if (this != &other) {
            buckets_ = std::move(other.buckets_);
            size_ = other.size_;
            hash_ = std::move(other.hash_);

            other.size_ = 0;
        }

        return *this;
    }

    ~HashMap() = default;

    void swap(HashMap& other) {
        buckets_.swap(other.buckets_);
        std::swap(size_, other.size_);
        std::swap(hash_, other.hash_);
    }

    bool insert(const K& key, const V& value) {
        if (buckets_.empty()) {
            buckets_.resize(8);
        }

        std::size_t index = hash_(key) % buckets_.size();

        Node* current = buckets_[index].get();

        while (current) {
            if (current->data.first == key) {
                return false;
            }

            current = current->next.get();
        }

        auto node = std::make_unique<Node>(key, value);
        node->next = std::move(buckets_[index]);
        buckets_[index] = std::move(node);

        ++size_;

        if (loadFactor() > 0.75) {
            rehash(buckets_.size() * 2);
        }

        return true;
    }

    V* find(const K& key) {
        if (buckets_.empty()) {
            return nullptr;
        }

        std::size_t index = hash_(key) % buckets_.size();

        Node* current = buckets_[index].get();

        while (current) {
            if (current->data.first == key) {
                return &current->data.second;
            }

            current = current->next.get();
        }

        return nullptr;
    }

    const V* find(const K& key) const {
        if (buckets_.empty()) {
            return nullptr;
        }

        std::size_t index = hash_(key) % buckets_.size();

        const Node* current = buckets_[index].get();

        while (current) {
            if (current->data.first == key) {
                return &current->data.second;
            }

            current = current->next.get();
        }

        return nullptr;
    }

    bool contains(const K& key) const {
        return find(key) != nullptr;
    }

    V& operator[](const K& key) {
        V* value = find(key);

        if (value) {
            return *value;
        }

        insert(key, V{});

        return *find(key);
    }

    V& at(const K& key) {
        V* value = find(key);

        if (!value) {
            throw std::out_of_range("Ключ не знайдено");
        }

        return *value;
    }

    const V& at(const K& key) const {
        const V* value = find(key);

        if (!value) {
            throw std::out_of_range("Ключ не знайдено");
        }

        return *value;
    }

    bool erase(const K& key) {
        if (buckets_.empty()) {
            return false;
        }

        std::size_t index = hash_(key) % buckets_.size();

        auto* current = &buckets_[index];

        while (*current) {
            if ((*current)->data.first == key) {
                *current = std::move((*current)->next);
                --size_;
                return true;
            }

            current = &((*current)->next);
        }

        return false;
    }

    std::size_t size() const {
        return size_;
    }

    std::size_t bucketCount() const {
        return buckets_.size();
    }

    std::size_t maxChainLength() const {
        std::size_t maxLength = 0;

        for (const auto& bucket : buckets_) {
            std::size_t length = 0;
            const Node* current = bucket.get();

            while (current) {
                ++length;
                current = current->next.get();
            }

            if (length > maxLength) {
                maxLength = length;
            }
        }

        return maxLength;
    }

    double loadFactor() const {
        if (buckets_.empty()) {
            return 0.0;
        }

        return static_cast<double>(size_) / buckets_.size();
    }

    void clear() {
        for (auto& bucket : buckets_) {
            bucket.reset();
        }

        size_ = 0;
    }

    bool empty() const {
        return size_ == 0;
    }

    Iterator begin() {
        for (std::size_t i = 0; i < buckets_.size(); ++i) {
            if (buckets_[i]) {
                return Iterator(this, i, buckets_[i].get());
            }
        }

        return end();
    }

    Iterator end() {
        return Iterator(this, buckets_.size(), nullptr);
    }

    ConstIterator begin() const {
        for (std::size_t i = 0; i < buckets_.size(); ++i) {
            if (buckets_[i]) {
                return ConstIterator(this, i, buckets_[i].get());
            }
        }

        return end();
    }

    ConstIterator end() const {
        return ConstIterator(this, buckets_.size(), nullptr);
    }

    ConstIterator cbegin() const {
        return begin();
    }

    ConstIterator cend() const {
        return end();
    }

    bool operator==(const HashMap& other) const {
        if (size_ != other.size_) {
            return false;
        }

        for (auto it = begin(); it != end(); ++it) {
            const V* value = other.find(it->first);

            if (!value || *value != it->second) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const HashMap& other) const {
        return !(*this == other);
    }

    friend std::ostream& operator<<(
        std::ostream& out,
        const HashMap& map
        ) {
        out << "{";

        bool first = true;

        for (auto it = map.begin(); it != map.end(); ++it) {
            if (!first) {
                out << ", ";
            }

            out << it->first << ": " << it->second;
            first = false;
        }

        out << "}";

        return out;
    }
};
