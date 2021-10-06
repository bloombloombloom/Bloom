#include "SupportedFeaturesQuery.hpp"

#include <QtCore/QString>

#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"

using namespace Bloom::DebugServers::Gdb::CommandPackets;

void SupportedFeaturesQuery::dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) {
    gdbRspDebugServer.handleGdbPacket(*this);
}

void SupportedFeaturesQuery::init() {
    /*
     * For qSupported packets, supported and unsupported GDB features are reported in the packet
     * data, where each GDB feature is separated by a semicolon.
     */

    // The "qSupported:" prefix occupies 11 bytes
    if (data.size() > 11) {
        auto packetData = QString::fromLocal8Bit(
            reinterpret_cast<const char*>(this->data.data() + 11),
            static_cast<int>(this->data.size() - 11)
        );

        auto featureList = packetData.split(";");
        auto gdbFeatureMapping = getGdbFeatureToNameMapping();

        for (int i = 0; i < featureList.size(); i++) {
            auto featureString = featureList.at(i);

            // We only care about supported features. Supported features will precede a '+' character.
            if (featureString[featureString.size() - 1] == '+') {
                featureString.remove('+');

                auto feature = gdbFeatureMapping.valueAt(featureString.toStdString());
                if (feature.has_value()) {
                    this->supportedFeatures.insert(static_cast<decltype(feature)::value_type>(feature.value()));
                }
            }
        }
    }
}
