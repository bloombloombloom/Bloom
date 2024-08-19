#pragma once

#include <QDomDocument>
#include <QDomElement>
#include <string>
#include <optional>
#include <functional>
#include <map>
#include <vector>
#include <set>

#include "PropertyGroup.hpp"
#include "AddressSpace.hpp"
#include "MemorySegment.hpp"
#include "MemorySegmentSection.hpp"
#include "PhysicalInterface.hpp"
#include "Module.hpp"
#include "RegisterGroup.hpp"
#include "Register.hpp"
#include "RegisterGroupReference.hpp"
#include "Peripheral.hpp"
#include "RegisterGroupInstance.hpp"
#include "Pad.hpp"
#include "Signal.hpp"
#include "Pinout.hpp"
#include "Pin.hpp"
#include "Variant.hpp"

#include "src/Targets/TargetFamily.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetPeripheralDescriptor.hpp"
#include "src/Targets/TargetPeripheralSignalDescriptor.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetBitFieldDescriptor.hpp"
#include "src/Targets/TargetPinoutDescriptor.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"
#include "src/Targets/TargetVariantDescriptor.hpp"
#include "src/Targets/TargetPhysicalInterface.hpp"

namespace Targets::TargetDescription
{
    /**
     * A target description file (TDF) is an XML file that describes a particular target. All supported targets come
     * with a target description file.
     *
     * Target description files are part of the Bloom codebase.
     * For target description files, see the directory "src/Targets/TargetDescriptionFiles/".
     *
     * During the build process, all target description files are copied to the distribution directory, ready
     * to be shipped with the Bloom binary.
     *
     * This class may be extended to further reflect a TDF that is specific to a particular target, target architecture
     * or target family. For example, the Targets::Microchip::Avr8::TargetDescriptionFile class inherits from this
     * class, to represent TDFs for AVR8 targets. The derived class provides access to additional data that is only
     * found in AVR8 TDFs (such as AVR target signature, AVR Family, etc).
     *
     * For more information of TDFs, see src/Targets/TargetDescription/README.md
     */
    class TargetDescriptionFile
    {
    public:
        /**
         * Will construct a TargetDescriptionFile instance from the XML of a target description file, the path to which
         * is given via xmlFilePath.
         *
         * @param xmlFilePath
         */
        explicit TargetDescriptionFile(const std::string& xmlFilePath);

        /**
         * Will construct a TargetDescriptionFile instance from pre-loaded XML.
         *
         * @param xml
         */
        explicit TargetDescriptionFile(const QDomDocument& xml);

        /**
         * Returns the target name extracted from the TDF.
         *
         * @return
         */
        [[nodiscard]] const std::string& getName() const;

        /**
         * Returns the target family extracted from the TDF.
         *
         * @return
         */
        [[nodiscard]] TargetFamily getFamily() const;

        [[nodiscard]] std::optional<std::string> tryGetVendorName() const;
        [[nodiscard]] const std::string& getVendorName() const;

        [[nodiscard]] std::optional<std::reference_wrapper<const PropertyGroup>> tryGetPropertyGroup(
            std::string_view keyStr
        ) const;
        [[nodiscard]] const PropertyGroup& getPropertyGroup(std::string_view keyStr) const;

        [[nodiscard]] std::optional<std::reference_wrapper<const Property>> tryGetProperty(
            std::string_view groupKey,
            std::string_view propertyKey
        ) const;

        [[nodiscard]] const Property& getProperty(std::string_view groupKey, std::string_view propertyKey) const;

        [[nodiscard]] std::optional<std::reference_wrapper<const AddressSpace>> tryGetAddressSpace(
            std::string_view key
        ) const;
        [[nodiscard]] const AddressSpace& getAddressSpace(std::string_view key) const;

        [[nodiscard]] std::optional<std::reference_wrapper<const MemorySegment>> tryGetMemorySegment(
            std::string_view addressSpaceKey,
            std::string_view segmentKey
        ) const;
        [[nodiscard]] const MemorySegment& getMemorySegment(
            std::string_view addressSpaceKey,
            std::string_view segmentKey
        ) const;

        [[nodiscard]] std::set<Targets::TargetPhysicalInterface> getPhysicalInterfaces() const;

        [[nodiscard]] std::optional<std::reference_wrapper<const Module>> tryGetModule(
            std::string_view key
        ) const;
        [[nodiscard]] const Module& getModule(std::string_view key) const;

        [[nodiscard]] std::optional<std::reference_wrapper<const Peripheral>> tryGetPeripheral(
            std::string_view key
        ) const;
        [[nodiscard]] const Peripheral& getPeripheral(std::string_view key) const;
        [[nodiscard]] std::set<const Peripheral*> getModulePeripherals(const std::string& moduleKey) const;
        [[nodiscard]] std::set<const Peripheral*> getGpioPeripherals() const;


        [[nodiscard]] std::optional<TargetMemorySegmentDescriptor> tryGetTargetMemorySegmentDescriptor(
            std::string_view addressSpaceKey,
            std::string_view segmentKey
        ) const;
        [[nodiscard]] TargetMemorySegmentDescriptor getTargetMemorySegmentDescriptor(
            std::string_view addressSpaceKey,
            std::string_view segmentKey
        ) const;

        [[nodiscard]] std::optional<TargetPeripheralDescriptor> tryGetTargetPeripheralDescriptor(
            std::string_view key
        ) const;
        [[nodiscard]] TargetPeripheralDescriptor getTargetPeripheralDescriptor(std::string_view key) const;

        [[nodiscard]] std::map<std::string, TargetAddressSpaceDescriptor> targetAddressSpaceDescriptorsByKey() const;
        [[nodiscard]] std::map<std::string, TargetPeripheralDescriptor> targetPeripheralDescriptorsByKey() const;
        [[nodiscard]] std::map<std::string, TargetPadDescriptor> targetPadDescriptorsByKey() const;
        [[nodiscard]] std::map<std::string, TargetPinoutDescriptor> targetPinoutDescriptorsByKey() const;
        [[nodiscard]] std::map<std::string, TargetVariantDescriptor> targetVariantDescriptorsByKey() const;
        [[nodiscard]] std::vector<TargetPeripheralDescriptor> gpioPortPeripheralDescriptors() const;

    protected:
        std::map<std::string, std::string> deviceAttributesByName;
        std::map<std::string, PropertyGroup, std::less<void>> propertyGroupsByKey;
        std::map<std::string, AddressSpace, std::less<void>> addressSpacesByKey;
        std::vector<PhysicalInterface> physicalInterfaces;
        std::map<std::string, Module, std::less<void>> modulesByKey;
        std::map<std::string, Peripheral, std::less<void>> peripheralsByKey;
        std::map<std::string, Pad, std::less<void>> padsByKey;
        std::map<std::string, Pinout, std::less<void>> pinoutsByKey;
        std::map<std::string, Variant, std::less<void>> variantsByKey;

        TargetDescriptionFile() = default;
        virtual ~TargetDescriptionFile() = default;

        TargetDescriptionFile(const TargetDescriptionFile& other) = default;
        TargetDescriptionFile(TargetDescriptionFile&& other) = default;

        TargetDescriptionFile& operator = (const TargetDescriptionFile& other) = default;
        TargetDescriptionFile& operator = (TargetDescriptionFile&& other) = default;

        void init(const std::string& xmlFilePath);
        void init(const QDomDocument& document);

        std::optional<std::reference_wrapper<const std::string>> tryGetDeviceAttribute(
            const std::string& attributeName
        ) const;
        const std::string& getDeviceAttribute(const std::string& attributeName) const;

        [[nodiscard]] std::set<std::string> getGpioPadKeys() const;

        static std::optional<std::string> tryGetAttribute(const QDomElement& element, const QString& attributeName);
        static std::string getAttribute(const QDomElement& element, const QString& attributeName);

        static PropertyGroup propertyGroupFromXml(const QDomElement& xmlElement);
        static Property propertyFromXml(const QDomElement& xmlElement);
        static AddressSpace addressSpaceFromXml(const QDomElement& xmlElement);
        static MemorySegment memorySegmentFromXml(const QDomElement& xmlElement);
        static MemorySegmentSection memorySegmentSectionFromXml(const QDomElement& xmlElement);
        static PhysicalInterface physicalInterfaceFromXml(const QDomElement& xmlElement);
        static Module moduleFromXml(const QDomElement& xmlElement);
        static RegisterGroup registerGroupFromXml(const QDomElement& xmlElement);
        static RegisterGroupReference registerGroupReferenceFromXml(const QDomElement& xmlElement);
        static Register registerFromXml(const QDomElement& xmlElement);
        static BitField bitFieldFromXml(const QDomElement& xmlElement);
        static Peripheral peripheralFromXml(const QDomElement& xmlElement);
        static RegisterGroupInstance registerGroupInstanceFromXml(const QDomElement& xmlElement);
        static Signal signalFromXml(const QDomElement& xmlElement);
        static Pad padFromXml(const QDomElement& xmlElement);
        static Pinout pinoutFromXml(const QDomElement& xmlElement);
        static Pin pinFromXml(const QDomElement& xmlElement);
        static Variant variantFromXml(const QDomElement& xmlElement);

        static TargetAddressSpaceDescriptor targetAddressSpaceDescriptorFromAddressSpace(
            const AddressSpace& addressSpace
        );

        static TargetMemorySegmentDescriptor targetMemorySegmentDescriptorFromMemorySegment(
            const MemorySegment& memorySegment,
            const AddressSpace& addressSpace
        );

        static TargetPeripheralDescriptor targetPeripheralDescriptorFromPeripheral(
            const Peripheral& peripheral,
            const Module& peripheralModule
        );

        static TargetPeripheralSignalDescriptor targetPeripheralSignalDescriptorFromSignal(const Signal& signal);

        static TargetRegisterGroupDescriptor targetRegisterGroupDescriptorFromRegisterGroup(
            const RegisterGroup& registerGroup,
            const Module& peripheralModule,
            const std::string& peripheralKey,
            const std::string& addressSpaceKey,
            TargetMemoryAddress baseAddress,
            const std::optional<std::string>& parentGroupAbsoluteKey,
            const std::optional<std::string>& description = std::nullopt,
            const std::optional<std::string>& keyOverride = std::nullopt,
            const std::optional<std::string>& nameOverride = std::nullopt
        );

        static TargetRegisterDescriptor targetRegisterDescriptorFromRegister(
            const Register& reg,
            const std::string& absoluteGroupKey,
            const std::string& peripheralKey,
            const std::string& addressSpaceKey,
            TargetMemoryAddress baseAddress
        );

        static TargetBitFieldDescriptor targetBitFieldDescriptorFromBitField(const BitField& bitField);

        static TargetPadDescriptor targetPadDescriptorFromPad(
            const Pad& pad,
            const std::set<std::string>& gpioPadKeys
        );

        static TargetPinoutDescriptor targetPinoutDescriptorFromPinout(const Pinout& pinout);
        static TargetPinDescriptor targetPinDescriptorFromPin(const Pin& pin);

        static TargetVariantDescriptor targetVariantDescriptorFromVariant(const Variant& variant);
    };
}
