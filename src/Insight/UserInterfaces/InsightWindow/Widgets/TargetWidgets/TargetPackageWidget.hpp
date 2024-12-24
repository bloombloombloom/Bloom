#pragma once

#include <QWidget>
#include <utility>
#include <unordered_map>
#include <optional>

#include "src/Targets/TargetVariantDescriptor.hpp"
#include "src/Targets/TargetPinoutDescriptor.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"
#include "src/Targets/TargetGpioPadState.hpp"
#include "src/Targets/TargetState.hpp"

#include "TargetPinWidget.hpp"

namespace Widgets::InsightTargetWidgets
{
    /**
     * Each custom target package widget should be derived from this class.
     */
    class TargetPackageWidget: public QWidget
    {
        Q_OBJECT

    public:
        TargetPackageWidget(
            const Targets::TargetVariantDescriptor& variantDescriptor,
            const Targets::TargetPinoutDescriptor& pinoutDescriptor,
            const Targets::TargetState& targetState,
            QWidget* parent
        );
        virtual void refreshPadStates(std::optional<std::function<void(void)>> callback = std::nullopt);

        QSize sizeHint() const override {
            return this->minimumSize();
        }

        QSize minimumSizeHint() const override {
            return this->sizeHint();
        }

    protected:
        const Targets::TargetVariantDescriptor& variantDescriptor;
        const Targets::TargetPinoutDescriptor& pinoutDescriptor;
        const Targets::TargetState& targetState;

        std::unordered_map<std::uint16_t, TargetPinWidget*> pinWidgetsByPosition;
        std::unordered_map<Targets::TargetPadId, TargetPinWidget*> pinWidgetsByPadId;
        Targets::TargetPadDescriptors padDescriptors;

        virtual void updatePadStates(const Targets::TargetGpioPadDescriptorAndStatePairs& padStatePairs);
        void onProgrammingModeEnabled();
        void onProgrammingModeDisabled();
    };
}
