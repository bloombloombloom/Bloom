#pragma once

#include "Command.hpp"

namespace Bloom::TargetController::Commands
{
    class EnableProgrammingMode: public Command
    {
    public:
        static constexpr CommandType type = CommandType::ENABLE_PROGRAMMING_MODE;
        static const inline std::string name = "EnableProgrammingMode";

        EnableProgrammingMode() = default;

        [[nodiscard]] CommandType getType() const override {
            return EnableProgrammingMode::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return false;
        }
    };
}
