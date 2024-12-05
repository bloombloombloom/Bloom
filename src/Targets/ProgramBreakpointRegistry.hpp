#pragma once

#include <cstdint>
#include <unordered_map>
#include <optional>
#include <functional>
#include <numeric>

#include "TargetBreakpoint.hpp"
#include "TargetMemory.hpp"

namespace Targets
{
    /**
     * Bookkeeping for program breakpoints.
     *
     * This template class will accept any type derived from the TargetProgramBreakpoint struct, to accommodate
     * additional context-specific breakpoint data.
     */
    template <typename BreakpointType>
        requires std::is_base_of_v<TargetProgramBreakpoint, BreakpointType>
    class ProgramBreakpointRegistryGeneric
    {
    public:
        std::unordered_map<TargetAddressSpaceId, std::unordered_map<TargetMemoryAddress, BreakpointType>> mapping;

        void insert(const BreakpointType& breakpoint) {
            this->mapping[breakpoint.addressSpaceDescriptor.id].emplace(breakpoint.address, breakpoint);
        }

        void remove(TargetAddressSpaceId addressSpaceId, TargetMemoryAddress address) {
            const auto addressMappingIt = this->mapping.find(addressSpaceId);
            if (addressMappingIt == this->mapping.end()) {
                return;
            }

            addressMappingIt->second.erase(address);
        }

        void remove(const TargetProgramBreakpoint& breakpoint) {
            this->remove(breakpoint.addressSpaceDescriptor.id, breakpoint.address);
        }

        std::optional<std::reference_wrapper<const BreakpointType>> find(
            TargetAddressSpaceId addressSpaceId,
            TargetMemoryAddress address
        ) const {
            const auto addressMappingIt = this->mapping.find(addressSpaceId);
            if (addressMappingIt == this->mapping.end()) {
                return std::nullopt;
            }

            const auto& addressMapping = addressMappingIt->second;
            const auto breakpointIt = addressMapping.find(address);
            if (breakpointIt == addressMapping.end()) {
                return std::nullopt;
            }

            return std::cref(breakpointIt->second);
        }

        std::optional<std::reference_wrapper<const BreakpointType>> find(
            const TargetProgramBreakpoint& breakpoint
        ) const {
            return this->find(breakpoint.addressSpaceDescriptor.id, breakpoint.address);
        }

        [[nodiscard]] bool contains(TargetAddressSpaceId addressSpaceId, TargetMemoryAddress address) const {
            const auto addressMappingIt = this->mapping.find(addressSpaceId);
            if (addressMappingIt == this->mapping.end()) {
                return false;
            }

            const auto& addressMapping = addressMappingIt->second;
            const auto breakpointIt = addressMapping.find(address);
            if (breakpointIt == addressMapping.end()) {
                return false;
            }

            return true;
        }

        bool contains(const BreakpointType& breakpoint) const {
            return this->contains(breakpoint.addressSpaceDescriptor.id, breakpoint.address);
        }

        [[nodiscard]] std::size_t size() const {
            return std::accumulate(
                this->mapping.begin(),
                this->mapping.end(),
                std::size_t{0},
                [] (std::size_t accumulatedSize, const decltype(this->mapping)::value_type& addressMappingPair) {
                    return accumulatedSize + addressMappingPair.second.size();
                }
            );
        }

        decltype(ProgramBreakpointRegistryGeneric::mapping)::const_iterator begin() const noexcept {
            return this->mapping.begin();
        }

        decltype(ProgramBreakpointRegistryGeneric::mapping)::const_iterator end() const noexcept {
            return this->mapping.end();
        }

        void clear() {
            this->mapping.clear();
        }
    };

    using ProgramBreakpointRegistry = ProgramBreakpointRegistryGeneric<TargetProgramBreakpoint>;
}
