#pragma once

#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <map>
#include <QString>
#include <QStringList>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ClickableWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TextInput.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Widgets
{
    struct AddressRangeTypeOption
    {
        QString text;
        MemoryRegionAddressInputType addressType;

        AddressRangeTypeOption(const QString& text, MemoryRegionAddressInputType addressType)
        : text(text), addressType(addressType) {};
    };

    class RegionItem: public ClickableWidget
    {
        Q_OBJECT

    public:
        RegionItem(
            const MemoryRegion& region,
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            QWidget *parent
        );
        void setSelected(bool selected);

        [[nodiscard]] QWidget* getFormWidget() const {
            return this->formWidget;
        }

        [[nodiscard]] QString getRegionNameInputValue() const {
            return this->nameInput->text();
        }

        [[nodiscard]] virtual const MemoryRegion& getMemoryRegion() const = 0;
        [[nodiscard]] virtual QStringList getValidationFailures() const;

    signals:
        void selected(RegionItem*);

    protected:
        static constexpr int NAME_LABEL_MAX_LENGTH = 34;

        const Targets::TargetMemoryDescriptor& memoryDescriptor;

        QWidget* formWidget = nullptr;
        TextInput* nameInput = nullptr;
        QComboBox* addressTypeInput = nullptr;
        TextInput* startAddressInput = nullptr;
        TextInput* endAddressInput = nullptr;
        TextInput* sizeInput = nullptr;

        virtual void initFormInputs();
        [[nodiscard]] MemoryRegionAddressInputType getSelectedAddressInputType() const;

        Targets::TargetMemoryAddressRange convertAbsoluteToRelativeAddressRange(
            const Targets::TargetMemoryAddressRange& absoluteAddressRange
        ) const;

        Targets::TargetMemoryAddressRange convertRelativeToAbsoluteAddressRange(
            const Targets::TargetMemoryAddressRange& relativeAddressRange
        ) const;

    private:
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* nameLabel = new QLabel(this);
        QLabel* typeLabel = new QLabel(this);
        QLabel* addressRangeLabel = new QLabel(this);
        QLabel* timeLabel = new QLabel(this);

        static inline const std::map<QString, AddressRangeTypeOption> addressRangeTypeOptionsByName = std::map<
            QString, AddressRangeTypeOption
        >({
            {"absolute", AddressRangeTypeOption("Absolute", MemoryRegionAddressInputType::ABSOLUTE)},
            {"relative", AddressRangeTypeOption("Relative", MemoryRegionAddressInputType::RELATIVE)},
        });

        void onAddressRangeInputChange();
        void onSizeInputChange();
        void onNameInputChange();
    };
}
