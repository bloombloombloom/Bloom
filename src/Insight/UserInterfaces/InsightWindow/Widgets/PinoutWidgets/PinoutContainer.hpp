#pragma once

#include <QGraphicsView>
#include <QResizeEvent>
#include <QKeyEvent>

#include "src/Targets/TargetDescriptor.hpp"

#include "PinoutScene.hpp"

namespace Widgets::PinoutWidgets
{
    class PinoutContainer: public QGraphicsView
    {
        Q_OBJECT

    public:
        PinoutScene* pinoutScene;

        PinoutContainer(const Targets::TargetDescriptor& targetDescriptor, QWidget* parent);
        void resizeEvent(QResizeEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;

    protected:
        const Targets::TargetDescriptor& targetDescriptor;
    };
}
