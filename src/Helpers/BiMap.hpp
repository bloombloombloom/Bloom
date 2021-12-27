#pragma once

#include <memory>
#include <map>

namespace Bloom
{
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
                auto insertResultPair = this->map.insert(std::pair<TypeA, TypeB>{it->first, it->second});
                this->flippedMap.insert(
                    std::pair<TypeB, typename std::unordered_map<TypeA, TypeB>::iterator>{
                        it->second,
                        insertResultPair.first
                    }
                );
            }
        }

        bool contains(TypeA key) const {
            return this->map.find(key) != this->map.end();
        }

        bool contains(TypeB key) const {
            return this->flippedMap.find(key) != this->flippedMap.end();
        }

        std::optional<TypeB> valueAt(TypeA key) const {
            std::optional<TypeB> output;

            if (this->contains(key)) {
                output = this->map.find(key)->second;
            }

            return output;
        }

        std::optional<TypeA> valueAt(TypeB key) const {
            std::optional<TypeA> output;

            if (this->contains(key)) {
                output = this->flippedMap.find(key)->second->first;
            }

            return output;
        }

        const TypeB& at(TypeA key) const {
            return this->map.at(key);
        }

        const TypeA& at(TypeB key) const {
            return this->flippedMap.at(key);
        }

        std::unordered_map<TypeA, TypeB> getMap() {
            return this->map;
        }

        void insert(const std::pair<TypeA, TypeB>& pair) {
            auto insertResultPair = this->map.insert(pair);
            this->flippedMap.insert(std::pair<TypeB, typename std::unordered_map<TypeA, TypeB>::iterator>(
                pair.second,
                insertResultPair.first
            ));
        }

    private:
        std::unordered_map<TypeA, TypeB> map = {};
        std::unordered_map<TypeB, typename std::unordered_map<TypeA, TypeB>::iterator> flippedMap = {};
    };
}
