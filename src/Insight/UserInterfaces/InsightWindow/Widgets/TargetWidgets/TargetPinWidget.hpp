#pragma once

#include <QWidget>
#include <utility>
#include <optional>
#include <functional>

#include "src/Targets/TargetPinDescriptor.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"
#include "src/Targets/TargetPinoutDescriptor.hpp"
#include "src/Targets/TargetGpioPadState.hpp"

#include "src/Services/StringService.hpp"

namespace Widgets::InsightTargetWidgets
{
    class TargetPinWidget: public QWidget
    {
        Q_OBJECT

    public:
        const Targets::TargetPinDescriptor& pinDescriptor;
        std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor;
        const Targets::TargetPinoutDescriptor& pinoutDescriptor;
        std::optional<Targets::TargetGpioPadState> padState;
        bool padStateChanged = false;

        TargetPinWidget(
            const Targets::TargetPinDescriptor& pinDescriptor,
            std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
            const Targets::TargetPinoutDescriptor& pinoutDescriptor,
            QWidget* parent
        );

        int getPinNumber() const {
            return Services::StringService::toUint8(this->pinDescriptor.position, 10);
        }

        const std::optional<Targets::TargetGpioPadState>& getPadState() const {
            return this->padState;
        }

        virtual void updatePadState(const Targets::TargetGpioPadState& padState) {
            this->padStateChanged = !this->padState.has_value() || this->padState != padState;
            this->padState = padState;
        }

        bool hasPadStateChanged() const {
            return this->padStateChanged;
        }

    public slots:
        virtual void onWidgetBodyClicked();
    };
}
