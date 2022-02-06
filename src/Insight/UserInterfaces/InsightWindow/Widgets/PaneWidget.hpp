#pragma once

#include <QWidget>

#include "PanelWidget.hpp"
#include "PaneState.hpp"

namespace Bloom::Widgets
{
    class PaneWidget: public QWidget
    {
        Q_OBJECT

    public:
        bool activated = false;
        PanelWidget* parentPanel = nullptr;

        explicit PaneWidget(PanelWidget* parent): QWidget(parent), parentPanel(parent) {};

        [[nodiscard]] PaneState getCurrentState() const {
            return PaneState(
                this->activated
            );
        }
    };
}
