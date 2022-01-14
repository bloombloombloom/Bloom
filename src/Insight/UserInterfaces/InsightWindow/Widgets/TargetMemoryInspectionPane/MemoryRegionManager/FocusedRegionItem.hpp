#pragma once

#include "RegionItem.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    struct DataTypeOption
    {
        QString text;
        MemoryRegionDataType dataType = MemoryRegionDataType::UNKNOWN;

        DataTypeOption(const QString& text, MemoryRegionDataType dataType)
        : text(text), dataType(dataType) {};
    };

    class FocusedRegionItem: public RegionItem
    {
        Q_OBJECT

    public:
        FocusedRegionItem(const FocusedMemoryRegion& region, QWidget *parent);

        [[nodiscard]] const MemoryRegion& getMemoryRegion() const override {
            return this->memoryRegion;
        };

        [[nodiscard]] virtual FocusedMemoryRegion generateFocusedMemoryRegionFromInput();

    protected:
        void initFormInputs() override;

    private:
        FocusedMemoryRegion memoryRegion;
        QComboBox* dataTypeInput = nullptr;

        static inline const std::map<QString, DataTypeOption> dataTypeOptionsByName = std::map<
            QString, DataTypeOption
        >({
              {"other", DataTypeOption("Other", MemoryRegionDataType::UNKNOWN)},
              {"unsigned_integer", DataTypeOption("Unsigned Integer", MemoryRegionDataType::UNSIGNED_INTEGER)},
              {"ascii", DataTypeOption("ASCII String", MemoryRegionDataType::ASCII_STRING)},
          });
    };
}
