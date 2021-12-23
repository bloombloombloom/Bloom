#include "RegionItem.hpp"

#include <QHBoxLayout>
#include <QString>

using namespace Bloom;
using namespace Bloom::Widgets;

RegionItem::RegionItem(
    const MemoryRegion& region,
    QWidget* parent
): ClickableWidget(parent) {
    this->setObjectName("region-item");
    this->setFixedHeight(50);
    this->layout->setContentsMargins(5, 5, 5, 0);

    this->timeLabel->setText(region.createdDate.toString("hh:mm"));
    this->timeLabel->setObjectName("time-label");

    auto regionName = region.name;
    regionName.truncate(RegionItem::NAME_LABEL_MAX_LENGTH);
    this->nameLabel->setText(regionName);
    this->nameLabel->setObjectName("name-label");

    this->typeLabel->setText(region.type == MemoryRegionType::FOCUSED ? "Focused" : "Excluded");
    this->typeLabel->setObjectName("type-label");

    auto addressRange = region.getAbsoluteAddressRange();
    this->addressRangeLabel->setText(
        "0x" + QString::number(addressRange.startAddress, 16).toUpper() + QString(" -> ")
            + "0x" + QString::number(addressRange.endAddress, 16).toUpper()
    );
    this->addressRangeLabel->setObjectName("address-label");

    auto* topLabelLayout = new QHBoxLayout();
    topLabelLayout->setSpacing(0);
    topLabelLayout->setContentsMargins(0, 0, 0, 0);
    topLabelLayout->addWidget(this->nameLabel, 0, Qt::AlignmentFlag::AlignLeft);
    topLabelLayout->addStretch(1);
    topLabelLayout->addWidget(this->typeLabel, 0, Qt::AlignmentFlag::AlignRight);

    auto* bottomLabelLayout = new QHBoxLayout();
    bottomLabelLayout->setSpacing(0);
    bottomLabelLayout->setContentsMargins(0, 0, 0, 0);
    bottomLabelLayout->addWidget(this->addressRangeLabel, 0, Qt::AlignmentFlag::AlignLeft);
    bottomLabelLayout->addStretch(1);
    bottomLabelLayout->addWidget(this->timeLabel, 0, Qt::AlignmentFlag::AlignRight);

    this->layout->setSpacing(5);
    this->layout->addLayout(topLabelLayout);
    this->layout->addLayout(bottomLabelLayout);
    this->layout->addStretch(1);

    auto onClick = [this] {
        this->setSelected(true);
    };

    QObject::connect(this, &ClickableWidget::clicked, this, onClick);
    QObject::connect(this, &ClickableWidget::rightClicked, this, onClick);

    this->setSelected(false);
}

void RegionItem::setSelected(bool selected) {
    this->setProperty("selected", selected);
    this->style()->unpolish(this);
    this->style()->polish(this);

    if (selected) {
        emit this->selected(this);
    }
}

QStringList RegionItem::getValidationFailures() const {
    auto validationFailures = QStringList();

    const auto& memoryDescriptor = this->getMemoryRegion().memoryDescriptor;

    if (this->nameInput->text().isEmpty()) {
        validationFailures.emplace_back("Missing region name.");
    }

    bool conversionOk = false;

    std::uint32_t startAddress = this->startAddressInput->text().toUInt(&conversionOk, 16);
    if (!conversionOk) {
        validationFailures.emplace_back("Invalid start address.");
    }

    std::uint32_t endAddress = this->endAddressInput->text().toUInt(&conversionOk, 16);
    if (!conversionOk) {
        validationFailures.emplace_back("Invalid end address.");
    }

    if (startAddress > endAddress) {
        validationFailures.emplace_back("The start address exceeds the end address.");
    }

    auto addressType = this->getSelectedAddressType();
    const auto memoryAddressRange = memoryDescriptor.addressRange;

    const auto memoryAddressRangeStr = QString(
        "0x" + QString::number(memoryAddressRange.startAddress, 16).toUpper() + QString(" -> ")
            + "0x" + QString::number(memoryAddressRange.endAddress, 16).toUpper()
    );

    std::uint32_t absoluteStartAddress = addressType == MemoryRegionAddressType::RELATIVE ?
        memoryAddressRange.startAddress + startAddress : startAddress;

    std::uint32_t absoluteEndAddress = addressType == MemoryRegionAddressType::RELATIVE ?
        memoryAddressRange.startAddress + endAddress : endAddress;

    if (absoluteStartAddress < memoryAddressRange.startAddress
        || absoluteStartAddress > memoryAddressRange.endAddress
    ) {
        validationFailures.emplace_back(
            "The start address is not within the absolute memory address range (" + memoryAddressRangeStr + ")."
        );
    }

    if (absoluteEndAddress < memoryAddressRange.startAddress || absoluteEndAddress > memoryAddressRange.endAddress) {
        validationFailures.emplace_back(
            "The end address not within the absolute memory address range (" + memoryAddressRangeStr + ")."
        );
    }

    return validationFailures;
}

void RegionItem::initFormInputs() {
    const auto& region = this->getMemoryRegion();

    this->nameInput = this->formWidget->findChild<TextInput*>("name-input");
    this->addressTypeInput = this->formWidget->findChild<QComboBox*>("address-type-input");
    this->startAddressInput = this->formWidget->findChild<TextInput*>("start-address-input");
    this->endAddressInput = this->formWidget->findChild<TextInput*>("end-address-input");
    this->sizeInput = this->formWidget->findChild<TextInput*>("size-input");

    this->nameInput->setText(region.name);
    this->sizeInput->setText(QString::number((region.addressRange.endAddress - region.addressRange.startAddress) + 1));

    for (const auto& [optionName, option] : RegionItem::addressRangeTypeOptionsByName) {
        this->addressTypeInput->addItem(option.text, optionName);
    }

    if (region.addressRangeType == MemoryRegionAddressType::RELATIVE) {
        this->addressTypeInput->setCurrentText(RegionItem::addressRangeTypeOptionsByName.at("relative").text);

        auto addressRange = region.getRelativeAddressRange();
        this->startAddressInput->setText("0x" + QString::number(addressRange.startAddress, 16).toUpper());
        this->endAddressInput->setText("0x" + QString::number(addressRange.endAddress, 16).toUpper());

    } else {
        this->addressTypeInput->setCurrentText(RegionItem::addressRangeTypeOptionsByName.at("absolute").text);

        auto addressRange = region.getAbsoluteAddressRange();
        this->startAddressInput->setText("0x" + QString::number(addressRange.startAddress, 16).toUpper());
        this->endAddressInput->setText("0x" + QString::number(addressRange.endAddress, 16).toUpper());
    }

    QObject::connect(this->startAddressInput, &QLineEdit::textEdited, this, &RegionItem::onAddressRangeInputChange);
    QObject::connect(this->endAddressInput, &QLineEdit::textEdited, this, &RegionItem::onAddressRangeInputChange);
    QObject::connect(this->sizeInput, &QLineEdit::textEdited, this, &RegionItem::onSizeInputChange);
    QObject::connect(this->nameInput, &QLineEdit::textEdited, this, &RegionItem::onNameInputChange);
}

MemoryRegionAddressType RegionItem::getSelectedAddressType() const {
    auto selectedAddressTypeOptionName = this->addressTypeInput->currentData().toString();
    if (RegionItem::addressRangeTypeOptionsByName.contains(selectedAddressTypeOptionName)) {
        return RegionItem::addressRangeTypeOptionsByName.at(selectedAddressTypeOptionName).addressType;
    }

    return MemoryRegionAddressType::ABSOLUTE;
}

void RegionItem::onAddressRangeInputChange() {
    bool startAddressConversionOk = false;
    bool endAddressConversionOk = false;
    std::uint32_t startAddress = this->startAddressInput->text().toUInt(&startAddressConversionOk, 16);
    std::uint32_t endAddress = this->endAddressInput->text().toUInt(&endAddressConversionOk, 16);

    if (startAddressConversionOk && endAddressConversionOk && startAddress <= endAddress) {
        this->sizeInput->setText(QString::number((endAddress - startAddress) + 1));
    }
}

void RegionItem::onSizeInputChange() {
    bool startAddressConversionOk = false;
    bool sizeConversionOk = false;
    std::uint32_t startAddress = this->startAddressInput->text().toUInt(&startAddressConversionOk, 16);
    std::uint32_t size = this->sizeInput->text().toUInt(&sizeConversionOk, 10);

    if (startAddressConversionOk && sizeConversionOk && size > 0) {
        this->endAddressInput->setText("0x" + QString::number((startAddress + size) - 1, 16).toUpper());
    }
}

void RegionItem::onNameInputChange() {
    auto newName = this->nameInput->text();
    newName.truncate(RegionItem::NAME_LABEL_MAX_LENGTH);
    this->nameLabel->setText(newName);
}
