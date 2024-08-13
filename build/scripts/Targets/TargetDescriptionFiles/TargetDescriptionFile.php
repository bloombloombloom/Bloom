<?php
namespace Targets\TargetDescriptionFiles;

use Targets\TargetPeripheral;
use Targets\TargetPhysicalInterface;
use Targets\TargetRegister;
use Targets\TargetRegisterBitField;
use Targets\TargetRegisterGroup;

require_once __DIR__ . "/PropertyGroup.php";
require_once __DIR__ . "/AddressSpace.php";
require_once __DIR__ . "/PhysicalInterface.php";
require_once __DIR__ . "/Peripheral.php";
require_once __DIR__ . "/RegisterGroupInstance.php";
require_once __DIR__ . "/Signal.php";
require_once __DIR__ . "/Module.php";
require_once __DIR__ . "/RegisterGroup.php";
require_once __DIR__ . "/RegisterGroupReference.php";
require_once __DIR__ . "/Register.php";
require_once __DIR__ . "/Pinout.php";
require_once __DIR__ . "/Variant.php";
require_once __DIR__ . "/TargetFamily.php";

require_once __DIR__ . "/../TargetPhysicalInterface.php";
require_once __DIR__ . "/../TargetPeripheral.php";
require_once __DIR__ . "/../TargetRegisterGroup.php";
require_once __DIR__ . "/../TargetRegister.php";
require_once __DIR__ . "/../TargetRegisterBitField.php";

class TargetDescriptionFile
{
    /** @var string[] */
    public array $deviceAttributesByName = [];

    /** @var PropertyGroup[] */
    public array $propertyGroups = [];

    /** @var AddressSpace[] */
    public array $addressSpaces = [];

    /** @var PhysicalInterface[] */
    public array $physicalInterfaces = [];

    /** @var Peripheral[] */
    public array $peripherals = [];

    /** @var Module[] */
    public array $modules = [];

    /** @var Pinout[] */
    public array $pinouts = [];

    /** @var Variant[] */
    public array $variants = [];

    public function getAdditionalDeviceAttributes(): array
    {
        return [];
    }

    public function getName(): ?string
    {
        return $this->deviceAttributesByName['name'] ?? null;
    }

    public function getFamily(): ?TargetFamily
    {
        return TargetFamily::tryFrom($this->deviceAttributesByName['family'] ?? null);
    }

    public function getConfigurationValue(): ?string
    {
        return $this->deviceAttributesByName['configuration-value'] ?? null;
    }

    public function getArchitecture(): ?string
    {
        return $this->deviceAttributesByName['architecture'] ?? null;
    }

    public function getVendor(): ?string
    {
        return $this->deviceAttributesByName['vendor'] ?? null;
    }

    /**
     * @return TargetPhysicalInterface[]
     */
    public function getSupportedPhysicalInterfaces(): array
    {
        return array_values(array_filter(array_map(
            fn (PhysicalInterface $interface) => TargetPhysicalInterface::tryFrom($interface->value),
            $this->physicalInterfaces
        )));
    }

    /**
     * @return TargetPhysicalInterface[]
     */
    public function getSupportedDebugPhysicalInterfaces(): array
    {
        return array_values(array_filter(
            $this->getSupportedPhysicalInterfaces(),
            fn (TargetPhysicalInterface $interface): bool => $interface->supportsDebugging()
        ));
    }

    public function getPropertyGroup(array|string $keys): ?PropertyGroup
    {
        if (is_string($keys)) {
            $keys = explode('.', $keys);
        }

        $firstLevelGroupKey = array_shift($keys);
        foreach ($this->propertyGroups as $propertyGroup) {
            if ($propertyGroup->key === $firstLevelGroupKey) {
                return !empty($keys) ? $propertyGroup->getSubgroup($keys) : $propertyGroup;
            }
        }

        return null;
    }

    public function getProperty(array|string $propertyGroupKeys, $propertyKey): ?Property
    {
        return ($propertyGroup = $this->getPropertyGroup($propertyGroupKeys)) instanceof PropertyGroup
            ? $propertyGroup->getProperty($propertyKey)
            : null;
    }

    public function getPropertyValue(array|string $propertyGroupKeys, $propertyKey): ?string
    {
        return ($property = $this->getProperty($propertyGroupKeys, $propertyKey)) instanceof Property
            ? $property->value
            : null;
    }

    public function getAddressSpace(string $key): ?AddressSpace
    {
        foreach ($this->addressSpaces as $addressSpace) {
            if ($addressSpace->key != $key) {
                continue;
            }

            return $addressSpace;
        }

        return null;
    }

    public function getMemorySegment(string $addressSpaceKey, string $memorySegmentKey): ?MemorySegment
    {
        if (($addressSpace = $this->getAddressSpace($addressSpaceKey)) instanceof AddressSpace) {
            foreach ($addressSpace->memorySegments as $memorySegment) {
                if ($memorySegment->key !== $memorySegmentKey) {
                    continue;
                }

                return $memorySegment;
            }
        }

        return null;
    }

    public function getMemorySegmentsFromAnyAddressSpace(string $key): ?MemorySegment
    {
        foreach ($this->addressSpaces as $addressSpace) {
            foreach ($addressSpace->memorySegments as $memorySegment) {
                if ($memorySegment->key === $key) {
                    return $memorySegment;
                }
            }
        }

        return null;
    }

    public function getPeripheral(string $key): ?Peripheral
    {
        foreach ($this->peripherals as $peripheral) {
            if ($peripheral->key === $key) {
                return $peripheral;
            }
        }

        return null;
    }

    /**
     * Gets all peripherals of a particular module
     *
     * @param string $moduleKey
     * @return Peripheral[]
     */
    public function getPeripheralsOfModule(string $moduleKey): array
    {
        return array_filter(
            $this->peripherals,
            fn (Peripheral $peripheral): bool => $peripheral->moduleKey === $moduleKey
        );
    }

    public function removePeripheral(string $key): void
    {
        $this->peripherals = array_filter(
            $this->peripherals,
            fn (Peripheral $peripheral): bool => $peripheral->key !== $key
        );
    }

    public function getModule(string $key): ?Module
    {
        foreach ($this->modules as $module) {
            if ($module->key === $key) {
                return $module;
            }
        }

        return null;
    }

    /**
     * Will rename the key of the given module, and update all associated peripherals.
     *
     * @param Module $module
     * @param string $newKey
     * @return void
     */
    public function renameModuleKey(Module $module, string $newKey): void
    {
        foreach ($this->getPeripheralsOfModule($module->key) as $peripheral) {
            $peripheral->moduleKey = $newKey;
        }

        $module->key = $newKey;
    }

    public function getPinout(string $key): ?Pinout
    {
        foreach ($this->pinouts as $pinout) {
            if ($pinout->key === $key) {
                return $pinout;
            }
        }

        return null;
    }

    /**
     * Returns all pinouts that are supported by the target package widget, in Bloom's Insight GUI.
     *
     * @return array
     */
    public function getInsightCompatiblePinouts(): array
    {
        return array_filter(
            $this->pinouts,
            fn (Pinout $pinout): bool => in_array(
                $pinout->type,
                [
                    PinoutType::DIP,
                    PinoutType::QFP,
                    PinoutType::QFN,
                    PinoutType::SOIC,
                    PinoutType::SSOP,
                ]
            )
        );
    }

    public function getTargetPeripheral(string $peripheralKey): ?TargetPeripheral
    {
        $peripheral = $this->getPeripheral($peripheralKey);
        return $peripheral instanceof Peripheral ? $this->targetPeripheralFromPeripheral($peripheral) : null;
    }

    public function getTargetRegisterGroup(string $peripheralKey, array|string $registerGroupKeys): ?TargetRegisterGroup
    {
        $peripheral = $this->getPeripheral($peripheralKey);
        return $peripheral instanceof Peripheral
            ? $this->targetPeripheralFromPeripheral($peripheral)->getRegisterGroup($registerGroupKeys)
            : null;
    }

    public function getTargetRegister(
        string $peripheralKey,
        array|string $groupKeys,
        array|string $registerKey
    ): ?TargetRegister {
        return ($peripheral = $this->getPeripheral($peripheralKey)) instanceof Peripheral
            ? $this->targetPeripheralFromPeripheral($peripheral)->getRegister($groupKeys, $registerKey)
            : null;
    }

    public function resolveRegisterGroupInstance(
        RegisterGroupInstance $registerGroupInstance,
        string $moduleKey
    ): ?RegisterGroup {
        $module = $this->getModule($moduleKey);
        return $module instanceof Module ? $module->getRegisterGroup($registerGroupInstance->registerGroupKey) : null;
    }

    public function resolveRegisterGroupReference(
        RegisterGroupReference $registerGroupReference,
        string $moduleKey
    ): ?RegisterGroup {
        $module = $this->getModule($moduleKey);
        return $module instanceof Module ? $module->getRegisterGroup($registerGroupReference->registerGroupKey) : null;
    }

    private function targetPeripheralFromPeripheral(Peripheral $peripheral): TargetPeripheral
    {
        $output = new TargetPeripheral($peripheral->name, []);

        foreach ($peripheral->registerGroupInstances as $registerGroupInstance) {
            $registerGroup = $this->resolveRegisterGroupInstance($registerGroupInstance, $peripheral->moduleKey);
            if ($registerGroup instanceof RegisterGroup) {
                $output->registerGroups[] = $this->targetRegisterGroupFromRegisterGroup(
                    $registerGroupInstance->addressSpaceKey,
                    $registerGroup,
                    $registerGroupInstance->offset ?? 0,
                    $peripheral->moduleKey,
                    $registerGroupInstance->key,
                    $registerGroupInstance->name
                );
            }
        }

        return $output;
    }

    private function targetRegisterGroupFromRegisterGroup(
        string $addressSpaceKey,
        RegisterGroup $registerGroup,
        int $addressOffset,
        string $moduleKey,
        ?string $keyOverride = null,
        ?string $nameOverride = null
    ): TargetRegisterGroup {
        $addressOffset += $registerGroup->offset ?? 0;
        $output = new TargetRegisterGroup(
            $keyOverride ?? $registerGroup->key,
            $nameOverride ?? $registerGroup->name,
            $addressSpaceKey,
            $addressOffset,
            [],
            []
        );

        foreach ($registerGroup->subgroups as $subgroup) {
            $output->subgroups[] = $this->targetRegisterGroupFromRegisterGroup(
                $addressSpaceKey,
                $subgroup,
                $addressOffset,
                $moduleKey
            );
        }

        foreach ($registerGroup->subgroupReferences as $subgroupReference) {
            $subgroup = $this->resolveRegisterGroupReference($subgroupReference, $moduleKey);

            if ($subgroup instanceof RegisterGroup) {
                $output->subgroups[] = $this->targetRegisterGroupFromRegisterGroup(
                    $addressSpaceKey,
                    $subgroup,
                    $addressOffset + $subgroupReference->offset,
                    $moduleKey,
                    $subgroupReference->key,
                    $subgroupReference->name
                );
            }
        }

        foreach ($registerGroup->registers as $register) {
            $output->registers[] = $this->targetRegisterFromRegister($register, $addressOffset, $addressSpaceKey);
        }

        return $output;
    }

    private function targetRegisterFromRegister(Register $register, int $addressOffset, ?string $addressSpaceKey)
    : TargetRegister {
        return new TargetRegister(
            $register->key,
            $register->name,
            $addressSpaceKey,
            $addressOffset + $register->offset,
            $register->size,
            $register->initialValue,
            $register->description,
            array_map(
                fn (BitField $bitField): TargetRegisterBitField => $this->targetRegisterBitFieldFromBitField($bitField),
                $register->bitFields
            )
        );
    }

    private function targetRegisterBitFieldFromBitField(BitField $bitField): TargetRegisterBitField
    {
        return new TargetRegisterBitField(
            $bitField->key,
            $bitField->name,
            $bitField->description,
            $bitField->mask,
            $bitField->access
        );
    }
}
