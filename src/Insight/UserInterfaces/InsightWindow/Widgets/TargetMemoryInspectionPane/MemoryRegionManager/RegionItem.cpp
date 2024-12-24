#include "RegionItem.hpp"

#include <QHBoxLayout>
#include <QString>

namespace Widgets
{
    using Targets::TargetMemoryAddressRange;

    RegionItem::RegionItem(
        const MemoryRegion& region,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        QWidget* parent
    )
        : ClickableWidget(parent)
        , memorySegmentDescriptor(memorySegmentDescriptor)
    {
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

        this->addressRangeLabel->setText(
            "0x" + QString::number(region.addressRange.startAddress, 16).toUpper() + QString(" -> ")
                + "0x" + QString::number(region.addressRange.endAddress, 16).toUpper()
        );
        this->addressRangeLabel->setObjectName("address-label");

        auto* topLabelLayout = new QHBoxLayout{};
        topLabelLayout->setSpacing(0);
        topLabelLayout->setContentsMargins(0, 0, 0, 0);
        topLabelLayout->addWidget(this->nameLabel, 0, Qt::AlignmentFlag::AlignLeft);
        topLabelLayout->addStretch(1);
        topLabelLayout->addWidget(this->typeLabel, 0, Qt::AlignmentFlag::AlignRight);

        auto* bottomLabelLayout = new QHBoxLayout{};
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

        // TODO: This is a horrible hack. It will be binned when I rewrite this widget to use ListView
        this->setStyleSheet(this->styleSheet());

        if (selected) {
            emit this->selected(this);
        }
    }

    QStringList RegionItem::getValidationFailures() const {
        auto validationFailures = QStringList{};

        if (this->nameInput->text().isEmpty()) {
            validationFailures.emplace_back("Missing region name.");
        }

        bool conversionOk = false;

        const auto startAddress = this->startAddressInput->text().toUInt(&conversionOk, 16);
        if (!conversionOk) {
            validationFailures.emplace_back("Invalid start address.");
        }

        const auto endAddress = this->endAddressInput->text().toUInt(&conversionOk, 16);
        if (!conversionOk) {
            validationFailures.emplace_back("Invalid end address.");
        }

        if (startAddress > endAddress) {
            validationFailures.emplace_back("The start address exceeds the end address.");
        }

        auto addressType = this->getSelectedAddressInputType();
        const auto memoryAddressRange = this->memorySegmentDescriptor.addressRange;

        const auto memoryAddressRangeStr = QString{
            "0x" + QString::number(memoryAddressRange.startAddress, 16).toUpper() + QString(" -> ")
                + "0x" + QString::number(memoryAddressRange.endAddress, 16).toUpper()
        };

        const auto absoluteAddressRange = addressType == AddressType::RELATIVE
            ? this->convertRelativeToAbsoluteAddressRange(TargetMemoryAddressRange{startAddress, endAddress})
            : TargetMemoryAddressRange{startAddress, endAddress};

        if (absoluteAddressRange.startAddress < memoryAddressRange.startAddress
            || absoluteAddressRange.startAddress > memoryAddressRange.endAddress
        ) {
            validationFailures.emplace_back(
                "The start address is not within the absolute memory address range (" + memoryAddressRangeStr + ")."
            );
        }

        if (absoluteAddressRange.endAddress < memoryAddressRange.startAddress
            || absoluteAddressRange.endAddress > memoryAddressRange.endAddress
        ) {
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
        this->sizeInput->setText(
            QString::number((region.addressRange.endAddress - region.addressRange.startAddress) + 1)
        );

        for (const auto& [optionName, option] : RegionItem::addressRangeTypeOptionsByName) {
            this->addressTypeInput->addItem(option.text, optionName);
        }

        if (region.addressRangeInputType == AddressType::RELATIVE) {
            auto relativeAddressRange = this->convertAbsoluteToRelativeAddressRange(region.addressRange);
            this->addressTypeInput->setCurrentText(RegionItem::addressRangeTypeOptionsByName.at("relative").text);

            this->startAddressInput->setText(
                "0x" + QString::number(relativeAddressRange.startAddress, 16).toUpper()
            );
            this->endAddressInput->setText(
                "0x" + QString::number(relativeAddressRange.endAddress, 16).toUpper()
            );

        } else {
            this->addressTypeInput->setCurrentText(RegionItem::addressRangeTypeOptionsByName.at("absolute").text);

            this->startAddressInput->setText(
                "0x" + QString::number(region.addressRange.startAddress, 16).toUpper()
            );
            this->endAddressInput->setText(
                "0x" + QString::number(region.addressRange.endAddress, 16).toUpper()
            );
        }

        QObject::connect(this->startAddressInput, &QLineEdit::textEdited, this, &RegionItem::onAddressRangeInputChange);
        QObject::connect(this->endAddressInput, &QLineEdit::textEdited, this, &RegionItem::onAddressRangeInputChange);
        QObject::connect(this->sizeInput, &QLineEdit::textEdited, this, &RegionItem::onSizeInputChange);
        QObject::connect(this->nameInput, &QLineEdit::textEdited, this, &RegionItem::onNameInputChange);
    }

    AddressType RegionItem::getSelectedAddressInputType() const {
        const auto selectedAddressTypeIt = RegionItem::addressRangeTypeOptionsByName.find(
            this->addressTypeInput->currentData().toString()
        );

        if (selectedAddressTypeIt != RegionItem::addressRangeTypeOptionsByName.end()) {
            return selectedAddressTypeIt->second.addressType;
        }

        return AddressType::ABSOLUTE;
    }

    TargetMemoryAddressRange RegionItem::convertAbsoluteToRelativeAddressRange(
        const TargetMemoryAddressRange& absoluteAddressRange
    ) const {
        return TargetMemoryAddressRange{
            absoluteAddressRange.startAddress - this->memorySegmentDescriptor.addressRange.startAddress,
            absoluteAddressRange.endAddress - this->memorySegmentDescriptor.addressRange.startAddress
        };
    }

    TargetMemoryAddressRange RegionItem::convertRelativeToAbsoluteAddressRange(
        const TargetMemoryAddressRange& relativeAddressRange
    ) const {
        return TargetMemoryAddressRange{
            relativeAddressRange.startAddress + this->memorySegmentDescriptor.addressRange.startAddress,
            relativeAddressRange.endAddress + this->memorySegmentDescriptor.addressRange.startAddress
        };
    }

    void RegionItem::onAddressRangeInputChange() {
        auto startAddressConversionOk = false;
        auto endAddressConversionOk = false;
        const auto startAddress = this->startAddressInput->text().toUInt(&startAddressConversionOk, 16);
        const auto endAddress = this->endAddressInput->text().toUInt(&endAddressConversionOk, 16);

        if (startAddressConversionOk && endAddressConversionOk && startAddress <= endAddress) {
            this->sizeInput->setText(QString::number((endAddress - startAddress) + 1));
        }
    }

    void RegionItem::onSizeInputChange() {
        auto startAddressConversionOk = false;
        auto sizeConversionOk = false;
        const auto startAddress = this->startAddressInput->text().toUInt(&startAddressConversionOk, 16);
        const auto size = this->sizeInput->text().toUInt(&sizeConversionOk, 10);

        if (startAddressConversionOk && sizeConversionOk && size > 0) {
            this->endAddressInput->setText("0x" + QString::number((startAddress + size) - 1, 16).toUpper());
        }
    }

    void RegionItem::onNameInputChange() {
        auto newName = this->nameInput->text();
        newName.truncate(RegionItem::NAME_LABEL_MAX_LENGTH);
        this->nameLabel->setText(newName);
    }
}
