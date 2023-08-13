#pragma once

#include <memory>
#include <unordered_map>
#include <optional>
#include <set>

/**
 * Simple bidirectional map
 *
 * This should only be used for small maps, with small elements (enums, string literals, etc).
 *
 * TODO: This needs some work - was written as a quick implementation with minimal requirements.
 * TODO: Add support for inserting/deleting elements (outside of construction).
 *
 * @tparam TypeA
 * @tparam TypeB
 */
template<typename TypeA, typename TypeB>
class BiMap
{
public:
    BiMap(std::initializer_list<std::pair<TypeA, TypeB>> elements) {
        for (auto it = elements.begin(); it != elements.end(); ++it) {
            this->map.insert(std::pair<TypeA, TypeB>{it->first, it->second});
            this->flippedMap.insert(std::pair<TypeB, TypeA>(it->second, it->first));
        }
    }

    bool contains(const TypeA& key) const {
        return this->map.find(key) != this->map.end();
    }

    bool contains(const TypeB& key) const {
        return this->flippedMap.find(key) != this->flippedMap.end();
    }

    auto find(const TypeA& key) const {
        const auto valueIt = this->map.find(key);
        return valueIt != this->map.end() ? std::optional(valueIt) : std::nullopt;
    }

    auto find(const TypeB& key) const {
        const auto valueIt = this->flippedMap.find(key);
        return valueIt != this->flippedMap.end() ? std::optional(valueIt) : std::nullopt;
    }

    std::optional<TypeB> valueAt(const TypeA& key) const {
        std::optional<TypeB> output;

        const auto valueIt = this->map.find(key);
        if (valueIt != this->map.end()) {
            output = valueIt->second;
        }

        return output;
    }

    std::optional<TypeA> valueAt(const TypeB& key) const {
        std::optional<TypeA> output;

        const auto valueIt = this->flippedMap.find(key);
        if (valueIt != this->flippedMap.end()) {
            output = valueIt->second;
        }

        return output;
    }

    const TypeB& at(const TypeA& key) const {
        return this->map.at(key);
    }

    const TypeA& at(const TypeB& key) const {
        return this->flippedMap.at(key);
    }

    [[nodiscard]] std::unordered_map<TypeA, TypeB> getMap() const {
        return this->map;
    }

    [[nodiscard]] std::set<TypeB> getKeys() const {
        auto keys = std::set<TypeB>();

        for (const auto& [key, value] : this->map) {
            keys.insert(key);
        }

        return keys;
    }

    [[nodiscard]] std::set<TypeB> getValues() const {
        auto values = std::set<TypeB>();

        for (const auto& [key, value] : this->map) {
            values.insert(value);
        }

        return values;
    }

    void insert(const std::pair<TypeA, TypeB>& pair) {
        auto insertResultPair = this->map.insert(pair);
        this->flippedMap.insert(std::pair<TypeB, TypeA>(pair.second, pair.first));
    }

private:
    std::unordered_map<TypeA, TypeB> map = {};
    std::unordered_map<TypeB, TypeA> flippedMap = {};
};
