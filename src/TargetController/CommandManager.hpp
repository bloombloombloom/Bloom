#pragma once

#include <memory>
#include <chrono>
#include <optional>

#include "Commands/Command.hpp"
#include "Responses/Response.hpp"
#include "Responses/Error.hpp"
#include "TargetControllerComponent.hpp"

#include "src/Exceptions/Exception.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::TargetController
{
    class CommandManager
    {
    public:
        template<class CommandType>
            requires
                std::is_base_of_v<Commands::Command, CommandType>
                && std::is_base_of_v<Responses::Response, typename CommandType::SuccessResponseType>
        auto sendCommandAndWaitForResponse(
            std::unique_ptr<CommandType> command,
            std::chrono::milliseconds timeout
        ) const {
            using SuccessResponseType = typename CommandType::SuccessResponseType;

            const auto commandId = command->id;
            Logger::debug(
                "Issuing " + CommandType::name + " command (ID: " + std::to_string(commandId) + ") to TargetController"
            );

            TargetControllerComponent::registerCommand(std::move(command));

            auto optionalResponse = TargetControllerComponent::waitForResponse(commandId, timeout);

            if (!optionalResponse.has_value()) {
                Logger::debug(
                    "Timed out whilst waiting for TargetController to respond to " + CommandType::name + " command"
                );
                throw Exceptions::Exception("Command timed out");
            }

            auto& response = optionalResponse.value();

            if (response->getType() == Responses::ResponseType::ERROR) {
                const auto errorResponse = dynamic_cast<Responses::Error*>(response.get());

                Logger::debug(
                    "TargetController returned error in response to " + CommandType::name + " command (ID: "
                        + std::to_string(commandId) + "). Error: " + errorResponse->errorMessage
                );
                throw Exceptions::Exception(errorResponse->errorMessage);
            }

            Logger::debug(
                "Delivering response for " + CommandType::name + " command (ID: " + std::to_string(commandId) + ")"
            );

            // Only downcast if the command's SuccessResponseType is not the generic Response type.
            if constexpr (!std::is_same_v<SuccessResponseType, Responses::Response>) {
                assert(response->getType() == SuccessResponseType::type);
                return std::unique_ptr<SuccessResponseType>(
                    dynamic_cast<SuccessResponseType*>(response.release())
                );

            } else {
                return std::move(response);
            }
        }
    };
}
