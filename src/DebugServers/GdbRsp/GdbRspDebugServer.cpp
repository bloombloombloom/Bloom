#include <sys/socket.h>
#include <sys/epoll.h>
#include <cstdint>

#include "GdbRspDebugServer.hpp"
#include "Exceptions/ClientDisconnected.hpp"
#include "Exceptions/ClientNotSupported.hpp"
#include "Exceptions/ClientCommunicationError.hpp"
#include "Exceptions/DebugSessionAborted.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::DebugServers::Gdb;
using namespace Bloom::DebugServers::Gdb::CommandPackets;
using namespace Bloom::DebugServers::Gdb::ResponsePackets;
using namespace Bloom::DebugServers::Gdb::Exceptions;
using namespace Bloom::Events;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetRegister;
using Bloom::Targets::TargetRegisterType;
using Bloom::Targets::TargetRegisterDescriptor;
using Bloom::Targets::TargetRegisterDescriptors;
using Bloom::Targets::TargetBreakpoint;

void GdbRspDebugServer::init() {
    auto ipAddress = this->debugServerConfig.jsonObject.find("ipAddress")->toString().toStdString();
    auto configPortJsonValue = this->debugServerConfig.jsonObject.find("port");
    auto configPortValue = configPortJsonValue->isString()
        ? static_cast<std::uint16_t>(configPortJsonValue->toString().toInt(nullptr, 10))
        : static_cast<std::uint16_t>(configPortJsonValue->toInt());

    if (!ipAddress.empty()) {
        this->listeningAddress = ipAddress;
    }

    if (configPortValue > 0) {
        this->listeningPortNumber = configPortValue;
    }

    this->socketAddress.sin_family = AF_INET;
    this->socketAddress.sin_port = htons(this->listeningPortNumber);

    if (::inet_pton(AF_INET, this->listeningAddress.c_str(), &(this->socketAddress.sin_addr)) == 0) {
        // Invalid IP address
        throw InvalidConfig("Invalid IP address provided in config file: (\"" + this->listeningAddress + "\")");
    }

    int socketFileDescriptor;

    if ((socketFileDescriptor = ::socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        throw Exception("Failed to create socket file descriptor.");
    }

    if (::setsockopt(
        socketFileDescriptor,
        SOL_SOCKET,
        SO_REUSEADDR,
       &this->enableReuseAddressSocketOption,
        sizeof(this->enableReuseAddressSocketOption)) < 0
    ) {
        Logger::error("Failed to set socket SO_REUSEADDR option.");
    }

    if (::bind(
        socketFileDescriptor,
        reinterpret_cast<const sockaddr*>(&(this->socketAddress)),
        sizeof(this->socketAddress)
        ) < 0
    ) {
        throw Exception("Failed to bind address. The selected port number ("
            + std::to_string(this->listeningPortNumber) + ") may be in use.");
    }

    this->serverSocketFileDescriptor = socketFileDescriptor;

    this->eventFileDescriptor = ::epoll_create(2);
    struct epoll_event event = {};
    event.events = EPOLLIN;
    event.data.fd = this->serverSocketFileDescriptor;

    if (::epoll_ctl(this->eventFileDescriptor, EPOLL_CTL_ADD, this->serverSocketFileDescriptor, &event) != 0) {
        throw Exception("Failed epoll_ctl server socket");
    }

    if (this->interruptEventNotifier != nullptr) {
        auto interruptFileDescriptor = this->interruptEventNotifier->getFileDescriptor();
        event.events = EPOLLIN;
        event.data.fd = interruptFileDescriptor;

        if (::epoll_ctl(this->eventFileDescriptor, EPOLL_CTL_ADD, interruptFileDescriptor, &event) != 0) {
            throw Exception("Failed epoll_ctl interrupt event fd");
        }
    }

    Logger::info("GDB RSP address: " + this->listeningAddress);
    Logger::info("GDB RSP port: " + std::to_string(this->listeningPortNumber));

    this->eventListener->registerCallbackForEventType<Events::TargetControllerStateReported>(
        std::bind(&GdbRspDebugServer::onTargetControllerStateReported, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::TargetExecutionStopped>(
        std::bind(&GdbRspDebugServer::onTargetExecutionStopped, this, std::placeholders::_1)
    );
}

void GdbRspDebugServer::close() {
    this->closeClientConnection();

    if (this->serverSocketFileDescriptor > 0) {
        ::close(this->serverSocketFileDescriptor);
    }
}

void GdbRspDebugServer::serve() {
    try {
        if (!this->clientConnection.has_value()) {
            Logger::info("Waiting for GDB RSP connection");

            do {
                this->waitForConnection();

            } while (!this->clientConnection.has_value());

            this->clientConnection->accept(this->serverSocketFileDescriptor);
            Logger::info("Accepted GDP RSP connection from " + this->clientConnection->getIpAddress());
            this->eventManager.triggerEvent(std::make_shared<Events::DebugSessionStarted>());

            /*
             * Before proceeding with a new debug session, we must ensure that the TargetController is able to
             * service it.
             */
            if (!this->targetControllerConsole.isTargetControllerInService()) {
                this->closeClientConnection();
                throw DebugSessionAborted("TargetController not in service");
            }
        }

        auto packets = this->clientConnection->readPackets();

        // Only process the last packet - any others will likely be duplicates from an impatient client
        if (!packets.empty()) {
            // Double-dispatch to appropriate handler
            packets.back()->dispatchToHandler(*this);
        }

    } catch (const ClientDisconnected&) {
        Logger::info("GDB RSP client disconnected");
        this->closeClientConnection();
        return;

    } catch (const ClientCommunicationError& exception) {
        Logger::error("GDB RSP client communication error - " + exception.getMessage() + " - closing connection");
        this->closeClientConnection();
        return;

    } catch (const ClientNotSupported& exception) {
        Logger::error("Invalid GDB RSP client - " + exception.getMessage() + " - closing connection");
        this->closeClientConnection();
        return;

    } catch (const DebugSessionAborted& exception) {
        Logger::warning("GDB debug session aborted - " + exception.getMessage());
        this->closeClientConnection();
        return;

    } catch (const DebugServerInterrupted&) {
        // Server was interrupted
        Logger::debug("GDB RSP interrupted");
        return;
    }
}

void GdbRspDebugServer::waitForConnection() {
    if (::listen(this->serverSocketFileDescriptor, 3) != 0) {
        throw Exception("Failed to listen on server socket");
    }

    std::array<struct epoll_event, 5> events = {};
    int eventCount = ::epoll_wait(
        this->eventFileDescriptor,
        events.data(),
        5,
        -1
    );

    if (eventCount > 0) {
        for (size_t i = 0; i < eventCount; i++) {
            auto fileDescriptor = events[i].data.fd;

            if (fileDescriptor == this->interruptEventNotifier->getFileDescriptor()) {
                // Interrupted
                this->interruptEventNotifier->clear();
                throw DebugServerInterrupted();
            }
        }

        this->clientConnection = Connection(this->interruptEventNotifier);
    }
}

void GdbRspDebugServer::onTargetControllerStateReported(Events::EventPointer<Events::TargetControllerStateReported> event) {
    if (event->state == TargetControllerState::SUSPENDED && this->clientConnection.has_value()) {
        Logger::warning("Terminating debug session - TargetController suspended unexpectedly");
        this->closeClientConnection();
    }
}

void GdbRspDebugServer::handleGdbPacket(CommandPacket& packet) {
    auto packetData = packet.getData();
    auto packetString = std::string(packetData.begin(), packetData.end());

    if (packetString[0] == '?') {
        // Status report
        this->clientConnection->writePacket(TargetStopped(Signal::TRAP));

    } else if (packetString[0] == 'D') {
        // Detach packet - there's not really anything we need to do here, so just respond with an OK
        this->clientConnection->writePacket(ResponsePacket({'O', 'K'}));

    } else if (packetString.find("qAttached") == 0) {
        Logger::debug("Handling qAttached");
        this->clientConnection->writePacket(ResponsePacket({1}));

    } else {
        Logger::debug("Unknown GDB RSP packet: " + packetString + " - returning empty response");

        // Respond with an empty packet
        this->clientConnection->writePacket(ResponsePacket({0}));
    }
}

void GdbRspDebugServer::onTargetExecutionStopped(EventPointer<Events::TargetExecutionStopped>) {
    if (this->clientConnection.has_value() && this->clientConnection->waitingForBreak) {
        this->clientConnection->writePacket(TargetStopped(Signal::TRAP));
        this->clientConnection->waitingForBreak = false;
    }
}

void GdbRspDebugServer::handleGdbPacket(CommandPackets::SupportedFeaturesQuery& packet) {
    Logger::debug("Handling QuerySupport packet");

    if (!packet.isFeatureSupported(Feature::HARDWARE_BREAKPOINTS)
        && !packet.isFeatureSupported(Feature::SOFTWARE_BREAKPOINTS)) {
        // All GDB clients are expected to support breakpoints!
        throw ClientNotSupported("GDB client does not support HW or SW breakpoints");
    }

    // Respond with a SupportedFeaturesResponse packet, listing all supported GDB features by Bloom
    auto response = ResponsePackets::SupportedFeaturesResponse({
       {Feature::SOFTWARE_BREAKPOINTS, std::nullopt},
       {Feature::PACKET_SIZE, std::to_string(this->clientConnection->getMaxPacketSize())},
    });

    this->clientConnection->writePacket(response);
}

void GdbRspDebugServer::handleGdbPacket(CommandPackets::ReadGeneralRegisters& packet) {
    Logger::debug("Handling ReadGeneralRegisters packet");

    try {
        auto descriptors = TargetRegisterDescriptors();

        if (packet.registerNumber.has_value()) {
            Logger::debug("Reading register number: " + std::to_string(packet.registerNumber.value()));
            descriptors.push_back(this->getRegisterDescriptorFromNumber(packet.registerNumber.value()));

        } else {
            // Read all descriptors
            auto descriptorMapping = this->getRegisterNumberToDescriptorMapping();
            for (auto& descriptor : descriptorMapping.getMap()) {
                descriptors.push_back(descriptor.second);
            }
        }

        auto registerSet = this->targetControllerConsole.readGeneralRegisters(descriptors);
        auto registerNumberToDescriptorMapping = this->getRegisterNumberToDescriptorMapping();

        /*
         * Remove any registers that are not mapped to GDB register numbers (as we won't know where to place
         * them in our response to GDB). All registers that are expected from the GDB client should be mapped
         * to register numbers.
         *
         * Registers that are not mapped to a GDB register number are presumed to be unknown to GDB, so GDB shouldn't
         * complain about not receiving them.
         */
        registerSet.erase(
            std::remove_if(
                registerSet.begin(),
                registerSet.end(),
                [&registerNumberToDescriptorMapping](const TargetRegister& reg) {
                    return !registerNumberToDescriptorMapping.contains(reg.descriptor);
                }
            ),
            registerSet.end()
        );

        /*
         * Sort each register by their respective GDB register number - this will leave us with a collection of
         * registers in the order expected by the GDB client.
         */
        std::sort(
            registerSet.begin(),
            registerSet.end(),
            [this, &registerNumberToDescriptorMapping](const TargetRegister& registerA, const TargetRegister& registerB) {
                return registerNumberToDescriptorMapping.valueAt(registerA.descriptor) <
                    registerNumberToDescriptorMapping.valueAt(registerB.descriptor);
            }
        );

        /*
         * Finally, implode the register values, convert to hexadecimal form and send to the GDB client.
         */
        auto registers = std::vector<unsigned char>();
        for (const auto& reg : registerSet) {
            registers.insert(registers.end(), reg.value.begin(), reg.value.end());
        }

        auto responseRegisters = Packet::dataToHex(registers);
        this->clientConnection->writePacket(
            ResponsePacket(std::vector<unsigned char>(responseRegisters.begin(), responseRegisters.end()))
        );

    } catch (const Exception& exception) {
        Logger::error("Failed to read general registers - " + exception.getMessage());
        this->clientConnection->writePacket(ResponsePacket({'E', '0', '1'}));
    }
}

void GdbRspDebugServer::handleGdbPacket(CommandPackets::WriteGeneralRegisters& packet) {
    Logger::debug("Handling WriteGeneralRegisters packet");

    try {
        auto registerDescriptor = this->getRegisterDescriptorFromNumber(packet.registerNumber);
        this->targetControllerConsole.writeGeneralRegisters({
            TargetRegister(registerDescriptor, packet.registerValue)
        });
        this->clientConnection->writePacket(ResponsePacket({'O', 'K'}));

    } catch (const Exception& exception) {
        Logger::error("Failed to write general registers - " + exception.getMessage());
        this->clientConnection->writePacket(ResponsePacket({'E', '0', '1'}));
    }
}

void GdbRspDebugServer::handleGdbPacket(CommandPackets::ContinueExecution& packet) {
    Logger::debug("Handling ContinueExecution packet");

    try {
        this->targetControllerConsole.continueTargetExecution(packet.fromProgramCounter);
        this->clientConnection->waitingForBreak = true;

    } catch (const Exception& exception) {
        Logger::error("Failed to continue execution on target - " + exception.getMessage());
        this->clientConnection->writePacket(ResponsePacket({'E', '0', '1'}));
    }
}

void GdbRspDebugServer::handleGdbPacket(CommandPackets::StepExecution& packet) {
    Logger::debug("Handling StepExecution packet");

    try {
        this->targetControllerConsole.stepTargetExecution(packet.fromProgramCounter);
        this->clientConnection->waitingForBreak = true;

    } catch (const Exception& exception) {
        Logger::error("Failed to step execution on target - " + exception.getMessage());
        this->clientConnection->writePacket(ResponsePacket({'E', '0', '1'}));
    }
}

void GdbRspDebugServer::handleGdbPacket(CommandPackets::ReadMemory& packet) {
    Logger::debug("Handling ReadMemory packet");

    try {
        auto memoryType = this->getMemoryTypeFromGdbAddress(packet.startAddress);
        auto startAddress = this->removeMemoryTypeIndicatorFromGdbAddress(packet.startAddress);
        auto memoryBuffer = this->targetControllerConsole.readMemory(memoryType, startAddress, packet.bytes);

        auto hexMemoryBuffer = Packet::dataToHex(memoryBuffer);
        this->clientConnection->writePacket(
            ResponsePacket(std::vector<unsigned char>(hexMemoryBuffer.begin(), hexMemoryBuffer.end()))
        );

    } catch (const Exception& exception) {
        Logger::error("Failed to read memory from target - " + exception.getMessage());
        this->clientConnection->writePacket(ResponsePacket({'E', '0', '1'}));
    }
}

void GdbRspDebugServer::handleGdbPacket(CommandPackets::WriteMemory& packet) {
    Logger::debug("Handling WriteMemory packet");

    try {
        auto memoryType = this->getMemoryTypeFromGdbAddress(packet.startAddress);
        auto startAddress = this->removeMemoryTypeIndicatorFromGdbAddress(packet.startAddress);
        this->targetControllerConsole.writeMemory(memoryType, startAddress, packet.buffer);

        this->clientConnection->writePacket(ResponsePacket({'O', 'K'}));

    } catch (const Exception& exception) {
        Logger::error("Failed to write memory two target - " + exception.getMessage());
        this->clientConnection->writePacket(ResponsePacket({'E', '0', '1'}));
    }
}

void GdbRspDebugServer::handleGdbPacket(CommandPackets::SetBreakpoint& packet) {
    Logger::debug("Handling SetBreakpoint packet");

    try {
        auto breakpoint = TargetBreakpoint();
        breakpoint.address = packet.address;
        this->targetControllerConsole.setBreakpoint(breakpoint);

        this->clientConnection->writePacket(ResponsePacket({'O', 'K'}));

    } catch (const Exception& exception) {
        Logger::error("Failed to set breakpoint on target - " + exception.getMessage());
        this->clientConnection->writePacket(ResponsePacket({'E', '0', '1'}));
    }
}

void GdbRspDebugServer::handleGdbPacket(CommandPackets::RemoveBreakpoint& packet) {
    Logger::debug("Removing breakpoint at address " + std::to_string(packet.address));

    try {
        auto breakpoint = TargetBreakpoint();
        breakpoint.address = packet.address;
        this->targetControllerConsole.removeBreakpoint(breakpoint);

        this->clientConnection->writePacket(ResponsePacket({'O', 'K'}));

    } catch (const Exception& exception) {
        Logger::error("Failed to remove breakpoint on target - " + exception.getMessage());
        this->clientConnection->writePacket(ResponsePacket({'E', '0', '1'}));
    }
}

void GdbRspDebugServer::handleGdbPacket(CommandPackets::InterruptExecution& packet) {
    Logger::debug("Handling InterruptExecution packet");

    try {
        this->targetControllerConsole.stopTargetExecution();
        this->clientConnection->writePacket(TargetStopped(Signal::INTERRUPTED));

    } catch (const Exception& exception) {
        Logger::error("Failed to interrupt execution - " + exception.getMessage());
        this->clientConnection->writePacket(ResponsePacket({'E', '0', '1'}));
    }
}
