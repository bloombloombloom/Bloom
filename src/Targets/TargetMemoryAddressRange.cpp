#include "TargetMemoryAddressRange.hpp"

#include <algorithm>
#include <cassert>

#include "src/Services/IntegerService.hpp"

namespace Targets
{
    TargetMemoryAddressRange::TargetMemoryAddressRange(
        TargetMemoryAddress startAddress,
        TargetMemoryAddress endAddress
    )
        : startAddress(startAddress)
        , endAddress(endAddress)
    {
        assert(this->startAddress <= this->endAddress);
    }

    bool TargetMemoryAddressRange::operator == (const TargetMemoryAddressRange& rhs) const {
        return this->startAddress == rhs.startAddress && this->endAddress == rhs.endAddress;
    }

    bool TargetMemoryAddressRange::operator < (const TargetMemoryAddressRange& rhs) const {
        return this->startAddress < rhs.startAddress;
    }

    TargetMemorySize TargetMemoryAddressRange::size() const {
        return this->endAddress - this->startAddress + 1;
    }

    bool TargetMemoryAddressRange::intersectsWith(const TargetMemoryAddressRange& other) const noexcept {
        return this->startAddress <= other.endAddress && other.startAddress <= this->endAddress;
    }

    TargetMemorySize TargetMemoryAddressRange::intersectingSize(const TargetMemoryAddressRange& other) const noexcept {
        return this->intersectsWith(other)
           ? std::min(this->endAddress, other.endAddress) - std::max(this->startAddress, other.startAddress) + 1
           : 0;
    }

    bool TargetMemoryAddressRange::contains(TargetMemoryAddress address) const noexcept {
        return address >= this->startAddress && address <= this->endAddress;
    }

    bool TargetMemoryAddressRange::contains(const TargetMemoryAddressRange& addressRange) const noexcept {
        return this->startAddress <= addressRange.startAddress && this->endAddress >= addressRange.endAddress;
    }

    std::set<Targets::TargetMemoryAddress> TargetMemoryAddressRange::addresses() const noexcept {
        auto addresses = std::set<Targets::TargetMemoryAddress>{};
        auto addressesIt = addresses.end();

        for (auto i = this->startAddress; i <= this->endAddress; ++i) {
            addressesIt = addresses.insert(addressesIt, i);
        }

        return addresses;
    }

    std::vector<TargetMemoryAddressRange> TargetMemoryAddressRange::blocks(TargetMemorySize blockSize) const noexcept {
        const auto startBlock = this->startAddress / blockSize;
        const auto endBlock = this->endAddress / blockSize;

        auto output = std::vector<TargetMemoryAddressRange>{};
        output.reserve(endBlock - startBlock + 1);

        for (auto block = startBlock; block <= endBlock; ++block) {
            const auto blockStartAddress = blockSize * block;
            const auto blockEndAddress = blockStartAddress + blockSize - 1;
            output.emplace_back(
                std::max(blockStartAddress, this->startAddress),
                std::min(blockEndAddress, this->endAddress)
            );
        }

        return output;
    }
}
