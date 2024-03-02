#include "TargetService.hpp"

namespace Services
{
    using Targets::BriefTargetDescriptor;

    const std::map<std::string, BriefTargetDescriptor>& TargetService::briefDescriptorsByConfigValue() {
        return TargetService::descriptorsByConfigValue;
    }

    std::optional<BriefTargetDescriptor> TargetService::briefDescriptor(const std::string& configValue) {
        const auto descriptorIt = TargetService::descriptorsByConfigValue.find(configValue);

        if (descriptorIt != TargetService::descriptorsByConfigValue.end()) {
            return descriptorIt->second;
        }

        return std::nullopt;
    }
}
