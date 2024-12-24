#pragma once

#include <QWidget>
#include <vector>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "../TargetPackageWidget.hpp"
#include "PinWidget.hpp"
#include "BodyWidget.hpp"

#include "src/Targets/TargetDescriptor.hpp"

namespace Widgets::InsightTargetWidgets::Qfp
{
    /**
     * QuadFlatPackageWidget implements a custom Qt widget for Quad-flat package variants.
     */
    class QuadFlatPackageWidget: public TargetPackageWidget
    {
        Q_OBJECT

    public:
        QuadFlatPackageWidget(
            const Targets::TargetVariantDescriptor& variantDescriptor,
            const Targets::TargetPinoutDescriptor& pinoutDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            QWidget* parent
        );

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    private:
        QVBoxLayout* layout = nullptr;
        QHBoxLayout* horizontalLayout = nullptr;
        QHBoxLayout* topPinLayout = nullptr;
        QVBoxLayout* rightPinLayout = nullptr;
        QHBoxLayout* bottomPinLayout = nullptr;
        QVBoxLayout* leftPinLayout = nullptr;
        BodyWidget* bodyWidget = nullptr;

        std::vector<PinWidget*> pinWidgets;

    };
}
