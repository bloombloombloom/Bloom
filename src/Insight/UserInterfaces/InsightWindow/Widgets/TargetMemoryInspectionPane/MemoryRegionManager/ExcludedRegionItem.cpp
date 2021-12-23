#include "ExcludedRegionItem.hpp"

#include <QFile>

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

using namespace Bloom;
using namespace Bloom::Widgets;

ExcludedRegionItem::ExcludedRegionItem(
    const ExcludedMemoryRegion& region,
    QWidget* parent
): memoryRegion(region), RegionItem(region, parent) {
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

ExcludedMemoryRegion ExcludedRegionItem::generateExcludedMemoryRegionFromInput() {
    this->memoryRegion.name = this->nameInput->text();
    this->memoryRegion.addressRange.startAddress = this->startAddressInput->text().toUInt(nullptr, 16);
    this->memoryRegion.addressRange.endAddress = this->endAddressInput->text().toUInt(nullptr, 16);
    this->memoryRegion.addressRangeType = this->getSelectedAddressType();

    return this->memoryRegion;
}
