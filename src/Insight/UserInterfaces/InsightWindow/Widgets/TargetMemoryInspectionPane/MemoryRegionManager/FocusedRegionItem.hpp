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
            : text(text)
            , dataType(dataType)
        {};
    };

    struct EndiannessOption
    {
        QString text;
        Targets::TargetMemoryEndianness endianness = Targets::TargetMemoryEndianness::LITTLE;

        EndiannessOption(const QString& text, Targets::TargetMemoryEndianness endianness)
            : text(text)
            , endianness(endianness)
        {};
    };

    class FocusedRegionItem: public RegionItem
    {
        Q_OBJECT

    public:
        FocusedRegionItem(
            const FocusedMemoryRegion& region,
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            QWidget *parent
        );

        [[nodiscard]] const FocusedMemoryRegion& getMemoryRegion() const override {
            return this->memoryRegion;
        };

        virtual void applyChanges();

    protected:
        void initFormInputs() override;

    private:
        FocusedMemoryRegion memoryRegion;
        QComboBox* dataTypeInput = nullptr;
        QComboBox* endiannessInput = nullptr;

        static const inline std::map<QString, DataTypeOption> dataTypeOptionsByName = std::map<
            QString, DataTypeOption
        >({
              {"other", DataTypeOption("Other", MemoryRegionDataType::UNKNOWN)},
              {"unsigned_integer", DataTypeOption("Unsigned Integer", MemoryRegionDataType::UNSIGNED_INTEGER)},
              {"signed_integer", DataTypeOption("Signed Integer", MemoryRegionDataType::SIGNED_INTEGER)},
              {"ascii", DataTypeOption("ASCII String", MemoryRegionDataType::ASCII_STRING)},
        });

        static const inline std::map<QString, EndiannessOption> endiannessOptionsByName = std::map<
            QString, EndiannessOption
        >({
              {"little", EndiannessOption("Little-endian", Targets::TargetMemoryEndianness::LITTLE)},
              {"big", EndiannessOption("Big-endian", Targets::TargetMemoryEndianness::BIG)},
        });
    };
}
