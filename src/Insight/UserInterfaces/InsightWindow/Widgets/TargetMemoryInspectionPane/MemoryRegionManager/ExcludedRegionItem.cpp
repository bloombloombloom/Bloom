#include "ExcludedRegionItem.hpp"

#include <QFile>

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

namespace Widgets
{
    ExcludedRegionItem::ExcludedRegionItem(
        const ExcludedMemoryRegion& region,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        QWidget* parent
    )
        : RegionItem(region, memorySegmentDescriptor, parent)
        , memoryRegion(region)
    {
        auto formUiFile = QFile{
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane"
                + "/MemoryRegionManager/UiFiles/ExcludedMemoryRegionForm.ui"
            )
        };

        if (!formUiFile.open(QFile::ReadOnly)) {
            throw Exceptions::Exception{"Failed to open excluded region item form UI file"};
        }

        auto uiLoader = UiLoader{this};
        this->formWidget = uiLoader.load(&formUiFile, this);

        this->initFormInputs();
    }

    void ExcludedRegionItem::applyChanges() {
        this->memoryRegion.name = this->nameInput->text();

        const auto inputAddressRange = Targets::TargetMemoryAddressRange{
            this->startAddressInput->text().toUInt(nullptr, 16),
            this->endAddressInput->text().toUInt(nullptr, 16)
        };
        this->memoryRegion.addressRangeInputType = this->getSelectedAddressInputType();
        this->memoryRegion.addressRange = this->memoryRegion.addressRangeInputType == AddressType::RELATIVE
            ? this->convertRelativeToAbsoluteAddressRange(inputAddressRange)
            : inputAddressRange;
    }
}
