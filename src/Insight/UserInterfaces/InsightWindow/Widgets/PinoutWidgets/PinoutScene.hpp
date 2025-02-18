#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <unordered_map>
#include <optional>
#include <functional>
#include <QMenu>

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetPinoutDescriptor.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"
#include "src/Targets/TargetGpioPadState.hpp"

#include "PinoutState.hpp"
#include "PinoutItem.hpp"
#include "PadLabels.hpp"
#include "LabelGroupInterface.hpp"

#include "src/Helpers/Pair.hpp"

namespace Widgets::PinoutWidgets
{
    class PinoutScene: public QGraphicsScene
    {
        Q_OBJECT

    public:
        explicit PinoutScene(const Targets::TargetDescriptor& targetDescriptor, QGraphicsView* parent);

        bool isPinoutSupported(const Targets::TargetPinoutDescriptor& pinoutDescriptor);
        void setPinout(const Targets::TargetPinoutDescriptor& pinoutDescriptor);
        void setDisabled(bool disabled);
        void refreshPadStates(const std::optional<std::function<void(void)>>& callback = std::nullopt);

    protected:
        QGraphicsView* parentView = nullptr;
        const Targets::TargetDescriptor& targetDescriptor;
        PinoutState pinoutState = {};
        PinoutItem* pinoutItem = nullptr;

        std::unordered_map<Targets::TargetPadId, PadLabels> padLabelsByPadId;
        Targets::TargetPadDescriptors gpioPadDescriptors;
        std::unordered_map<Targets::TargetPadId, Targets::TargetGpioPadState> gpioPadStatesByPadId;

        void adjustSize();
        void populatePadLabels(
            const std::vector<
                Pair<const Targets::TargetPadDescriptor&, LabelGroupInterface*>
            >& padDescriptorLabelGroupPairs
        );
        void updatePadStates(const Targets::TargetGpioPadDescriptorAndStatePairs& padStatePairs);
        void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
        QMenu* contextMenuForPad(const Targets::TargetPadDescriptor& padDescriptor);

        void toggleOutputPad(const Targets::TargetPadDescriptor& padDescriptor);
    };
}
