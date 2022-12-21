#pragma once

#include <QWidget>
#include <set>
#include <vector>
#include <QShowEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QAction>

#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PushButton.hpp"
#include "RegionItem.hpp"
#include "FocusedRegionItem.hpp"
#include "ExcludedRegionItem.hpp"

namespace Bloom::Widgets
{
    class MemoryRegionManagerWindow: public QWidget
    {
        Q_OBJECT

    public:
        explicit MemoryRegionManagerWindow(
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            QWidget* parent = nullptr
        );

        void refreshRegions();

    signals:
        void changesApplied();

    protected:
        void showEvent(QShowEvent* event) override;

    private:
        const Targets::TargetMemoryDescriptor& memoryDescriptor;
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;

        QWidget* container = nullptr;
        QWidget* regionSelector = nullptr;

        PushButton* applyButton = nullptr;
        PushButton* helpButton = nullptr;
        PushButton* closeButton = nullptr;

        SvgToolButton* addRegionButton = nullptr;
        QAction* addFocusedRegionMenuAction = nullptr;
        QAction* addExcludedRegionMenuAction = nullptr;
        SvgToolButton* removeRegionButton = nullptr;

        QScrollArea* regionItemScrollArea = nullptr;
        QWidget* regionItemScrollAreaViewport = nullptr;
        QVBoxLayout* regionItemScrollAreaViewportLayout = nullptr;

        QStackedLayout* stackedFormLayout = nullptr;

        std::set<FocusedRegionItem*> focusedRegionItems;
        std::set<ExcludedRegionItem*> excludedRegionItems;

        RegionItem* selectedRegion = nullptr;

        void clearRegions();
        void sortRegionItems();
        FocusedRegionItem* addFocusedRegion(const FocusedMemoryRegion& region);
        ExcludedRegionItem* addExcludedRegion(const ExcludedMemoryRegion& region);
        void onRegionSelected(RegionItem* selectedRegion);
        void onNewFocusedRegionTrigger();
        void onNewExcludedRegionTrigger();
        void onDeleteRegionTrigger();
        void applyChanges();
        void openHelpPage();
    };
}
