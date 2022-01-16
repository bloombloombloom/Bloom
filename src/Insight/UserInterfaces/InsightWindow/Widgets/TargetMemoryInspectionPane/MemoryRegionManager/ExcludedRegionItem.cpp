#include "ExcludedRegionItem.hpp"

#include <QFile>

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

using namespace Bloom;
using namespace Bloom::Widgets;

using Targets::TargetMemoryAddressRange;

ExcludedRegionItem::ExcludedRegionItem(
    const ExcludedMemoryRegion& region,
    const Targets::TargetMemoryDescriptor& memoryDescriptor,
    QWidget* parent
): memoryRegion(region), RegionItem(region, memoryDescriptor, parent) {
    auto formUiFile = QFile(
        QString::fromStdString(Paths::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane"
                + "/MemoryRegionManager/UiFiles/ExcludedMemoryRegionForm.ui"
        )
    );

    if (!formUiFile.open(QFile::ReadOnly)) {
        throw Bloom::Exceptions::Exception("Failed to open excluded region item form UI file");
    }

    auto uiLoader = UiLoader(this);
    this->formWidget = uiLoader.load(&formUiFile, this);

    this->initFormInputs();
}

void ExcludedRegionItem::applyChanges() {
    this->memoryRegion.name = this->nameInput->text();

    const auto inputAddressRange = TargetMemoryAddressRange(
        this->startAddressInput->text().toUInt(nullptr, 16),
        this->endAddressInput->text().toUInt(nullptr, 16)
    );
    this->memoryRegion.addressRangeType = this->getSelectedAddressType();
    this->memoryRegion.addressRange = this->memoryRegion.addressRangeType == MemoryRegionAddressType::RELATIVE ?
        this->convertRelativeToAbsoluteAddressRange(inputAddressRange) : inputAddressRange;
}
