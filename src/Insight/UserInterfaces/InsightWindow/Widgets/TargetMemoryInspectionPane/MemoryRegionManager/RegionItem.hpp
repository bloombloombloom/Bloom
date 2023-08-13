#pragma once

#include <QVBoxLayout>
#include <QComboBox>
#include <map>
#include <QString>
#include <QStringList>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ClickableWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TextInput.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Widgets
{
    struct AddressRangeTypeOption
    {
        QString text;
        AddressType addressType;

        AddressRangeTypeOption(const QString& text, AddressType addressType)
            : text(text)
            , addressType(addressType)
        {};
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
        [[nodiscard]] AddressType getSelectedAddressInputType() const;

        Targets::TargetMemoryAddressRange convertAbsoluteToRelativeAddressRange(
            const Targets::TargetMemoryAddressRange& absoluteAddressRange
        ) const;

        Targets::TargetMemoryAddressRange convertRelativeToAbsoluteAddressRange(
            const Targets::TargetMemoryAddressRange& relativeAddressRange
        ) const;

    private:
        QVBoxLayout* layout = new QVBoxLayout(this);
        Label* nameLabel = new Label(this);
        Label* typeLabel = new Label(this);
        Label* addressRangeLabel = new Label(this);
        Label* timeLabel = new Label(this);

        static const inline std::map<QString, AddressRangeTypeOption> addressRangeTypeOptionsByName = std::map<
            QString, AddressRangeTypeOption
        >({
            {"absolute", AddressRangeTypeOption("Absolute", AddressType::ABSOLUTE)},
            {"relative", AddressRangeTypeOption("Relative", AddressType::RELATIVE)},
        });

        void onAddressRangeInputChange();
        void onSizeInputChange();
        void onNameInputChange();
    };
}
