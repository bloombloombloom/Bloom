<?php
namespace Targets\TargetDescriptionFiles\Services;

use Targets\TargetDescriptionFiles\Pin;
use Targets\TargetDescriptionFiles\TargetDescriptionFile;
use Targets\TargetDescriptionFiles\AddressSpace;
use Targets\TargetDescriptionFiles\BitField;
use Targets\TargetDescriptionFiles\MemorySegment;
use Targets\TargetDescriptionFiles\MemorySegmentSection;
use Targets\TargetDescriptionFiles\Module;
use Targets\TargetDescriptionFiles\Peripheral;
use Targets\TargetDescriptionFiles\RegisterGroupInstance;
use Targets\TargetDescriptionFiles\Property;
use Targets\TargetDescriptionFiles\PropertyGroup;
use Targets\TargetDescriptionFiles\Register;
use Targets\TargetDescriptionFiles\RegisterGroup;
use Targets\TargetDescriptionFiles\RegisterGroupReference;
use Targets\TargetDescriptionFiles\Signal;
use Targets\TargetDescriptionFiles\Pinout;
use Targets\TargetDescriptionFiles\PinoutType;
use Targets\TargetDescriptionFiles\Variant;

require_once __DIR__ . '/../TargetDescriptionFile.php';

class ValidationService
{
    public function validateTdf(TargetDescriptionFile $tdf): array
    {
        $failures = [];

        if (empty($tdf->getName())) {
            $failures[] = 'Target name not found';
        }

        if (str_contains($tdf->getName(), ' ')) {
            $failures[] = 'Target name cannot contain whitespaces';
        }

        if (empty($tdf->getFamily())) {
            $failures[] = 'Missing/invalid target family';
        }

        if (empty($tdf->getConfigurationValue())) {
            $failures[] = 'Missing configuration value';
        }

        if (empty($tdf->variants)) {
            $failures[] = 'Missing target variants';
        }

        $processedPropertyGroupKeys = [];
        foreach ($tdf->propertyGroups as $propertyGroup) {
            $failures = array_merge($failures, $this->validatePropertyGroup($propertyGroup));

            if ($propertyGroup->key !== null && in_array($propertyGroup->key, $processedPropertyGroupKeys)) {
                $failures[] = 'Duplicate property group key ("' . $propertyGroup->key . '") detected';
            }

            $processedPropertyGroupKeys[] = $propertyGroup->key;
        }

        if (empty($tdf->addressSpaces)) {
            $failures[] = 'Missing address spaces';

        } elseif (count($tdf->addressSpaces) > 256) {
            /*
             * We store address space IDs in an std::uint8_t - which should be more than enough (I don't think we'll
             * ever support a target with more than 256 address spaces).
             */
            $failures[] = 'Too many address spaces (' . count($tdf->addressSpaces) . ')';
        }

        $processedAddressSpaceKeys = [];
        foreach ($tdf->addressSpaces as $addressSpace) {
            $failures = array_merge($failures, $this->validateAddressSpace($addressSpace));

            if ($addressSpace->key !== null && in_array($addressSpace->key, $processedAddressSpaceKeys)) {
                $failures[] = 'Duplicate address space key ("' . $addressSpace->key . '") detected';
            }

            $processedAddressSpaceKeys[] = $addressSpace->key;
        }

        if (empty($tdf->modules)) {
            $failures[] = 'Missing modules';
        }

        $processedModuleKeys = [];
        foreach ($tdf->modules as $module) {
            $failures = array_merge($failures, $this->validateModule($module, $tdf));

            if ($module->key !== null && in_array($module->key, $processedModuleKeys)) {
                $failures[] = 'Duplicate module key ("' . $module->key . '") detected';
            }

            $processedModuleKeys[] = $module->key;
        }

        if (empty($tdf->getModule('gpio_port'))) {
            $failures[] = 'Missing GPIO port module';
        }

        if (empty($tdf->modules)) {
            $failures[] = 'Missing peripherals';
        }

        $processedPeripheralKeys = [];
        foreach ($tdf->peripherals as $peripheral) {
            $failures = array_merge($failures, $this->validatePeripheral($peripheral, $tdf));

            if ($peripheral->key !== null && in_array($peripheral->key, $processedPeripheralKeys)) {
                $failures[] = 'Duplicate peripheral key ("' . $peripheral->key . '") detected';
            }

            if ($peripheral->moduleKey !== null && $tdf->getModule($peripheral->moduleKey) === null) {
                $failures[] = 'Invalid module key ("' . $peripheral->moduleKey . '") on peripheral "'
                    . $peripheral->key . '"';
            }

            $processedPeripheralKeys[] = $peripheral->key;
        }

        if (empty($tdf->getPeripheralsOfModule('gpio_port'))) {
            $failures[] = 'Missing GPIO port peripherals';
        }

        if (empty($tdf->pinouts)) {
            $failures[] = 'Missing pinouts';
        }

        $processedPinoutKeys = [];
        foreach ($tdf->pinouts as $pinout) {
            $failures = array_merge($failures, $this->validatePinout($pinout));

            if ($pinout->key !== null && in_array($pinout->key, $processedPinoutKeys)) {
                $failures[] = 'Duplicate pinout key ("' . $pinout->key . '") detected';
            }

            $processedPinoutKeys[] = $pinout->key;
        }

        $processedVariantNames = [];
        foreach ($tdf->variants as $variant) {
            $failures = array_merge($failures, $this->validateVariant($variant, $tdf));

            if ($variant->name !== null && in_array($variant->name, $processedVariantNames)) {
                $failures[] = 'Duplicate variant name ("' . $variant->name . '") detected';
            }

            $processedVariantNames[] = $variant->name;
        }

        return $failures;
    }

    protected function validatePropertyGroup(PropertyGroup $propertyGroup): array
    {
        $failures = [];

        if (empty($propertyGroup->key)) {
            $failures[] = 'Missing key';
        }

        if (!mb_check_encoding((string) $propertyGroup->key, 'ASCII')) {
            $failures[] = 'Key contains non ASCII characters';
        }

        if (str_contains((string) $propertyGroup->key, ' ')) {
            $failures[] = 'Key contains at least one white character';
        }

        if (str_contains((string) $propertyGroup->key, '.')) {
            $failures[] = 'Key contains at least one period (".") character';
        }

        if (empty($propertyGroup->subPropertyGroups) && empty($propertyGroup->properties)) {
            $failures[] = 'Empty property group';
        }

        $processedSubPropertyGroupKeys = [];
        foreach ($propertyGroup->subPropertyGroups as $subPropertyGroup) {
            $failures = array_merge($failures, $this->validatePropertyGroup($subPropertyGroup));

            if ($subPropertyGroup->key !== null && in_array($subPropertyGroup->key, $processedSubPropertyGroupKeys)) {
                $failures[] = 'Duplicate property group key ("' . $subPropertyGroup->key . '") detected';
            }

            $processedSubPropertyGroupKeys[] = $subPropertyGroup->key;
        }

        $processedPropertyKeys = [];
        foreach ($propertyGroup->properties as $property) {
            $failures = array_merge($failures, $this->validateProperty($property));

            if ($property->key !== null && in_array($property->key, $processedPropertyKeys)) {
                $failures[] = 'Duplicate property key ("' . $property->key . '") detected';
            }

            $processedPropertyKeys[] = $property->key;
        }

        return array_map(
            fn (string $failure): string => 'Property group (id: "' . $propertyGroup->key . '") validation failure: '
                . $failure,
            $failures
        );
    }

    protected function validateProperty(Property $property): array
    {
        $failures = [];

        if (empty($property->key)) {
            $failures[] = 'Missing key';
        }

        if (!mb_check_encoding((string) $property->key, 'ASCII')) {
            $failures[] = 'Key contains non ASCII characters';
        }

        if (str_contains((string) $property->key, ' ')) {
            $failures[] = 'Key contains at least white space character';
        }

        if (str_contains((string) $property->key, '.')) {
            $failures[] = 'Key contains at least one period (".") character';
        }

        if ($property->value === null) {
            $failures[] = 'Missing value';
        }

        return array_map(
            fn (string $failure): string => 'Property (key: "' . $property->key . '") validation failure: ' . $failure,
            $failures
        );
    }

    protected function validateAddressSpace(AddressSpace $addressSpace): array
    {
        $failures = [];

        if (empty($addressSpace->key)) {
            $failures[] = 'Missing key';
        }

        if ($addressSpace->startAddress === null) {
            $failures[] = 'Missing start address';

        } elseif ($addressSpace->startAddress > 0xFFFFFFFF) {
            $failures[] = 'Start address exceeds 32-bit unsigned integer';
        }

        if ($addressSpace->size === null) {
            $failures[] = 'Missing size';

        } elseif ($addressSpace->size > 0xFFFFFFFF) {
            $failures[] = 'Size exceeds 32-bit unsigned integer';
        }

        if (empty($addressSpace->memorySegments)) {
            $failures[] = 'Missing memory segments - all address spaces must contain at least one memory segment';
        }

        $processedSegmentKeys = [];
        foreach ($addressSpace->memorySegments as $segment) {
            $failures = array_merge($failures, $this->validateMemorySegment($segment));

            if ($segment->key !== null && in_array($segment->key, $processedSegmentKeys)) {
                $failures[] = 'Duplicate segment key ("' . $segment->key . '") detected';
            }

            foreach ($addressSpace->memorySegments as $segmentOther) {
                if ($segment->key === $segmentOther->key) {
                    continue;
                }

                if ($segment->intersectsWith($segmentOther)) {
                    $failures[] = 'Segment "' . $segment->key . '" overlaps with segment "' . $segmentOther->key . '"';
                }
            }

            $processedSegmentKeys[] = $segment->key;
        }

        $totalSegmentSize = $addressSpace->totalSegmentSize();
        if ($totalSegmentSize > $addressSpace->size) {
            $failures[] = 'Total size of all contained segments (' . $totalSegmentSize . ' bytes) exceeds the total'
                . ' size of the address space (' . $addressSpace->size . ' bytes)';
        }

        return array_map(
            fn (string $failure): string => 'Address space (key: "' . $addressSpace->key . '") validation failure: '
                . $failure,
            $failures
        );
    }

    protected function validateMemorySegment(MemorySegment $segment): array
    {
        $failures = [];

        if (empty($segment->key)) {
            $failures[] = 'Missing key';
        }

        if (!preg_match('/^[a-z_]+$/',$segment->key)) {
            $failures[] = 'Key must contain only lowercase letters';
        }

        if (!mb_check_encoding((string) $segment->key, 'ASCII')) {
            $failures[] = 'Key must contain only ASCII characters';
        }

        if (empty($segment->name)) {
            $failures[] = 'Missing name';
        }

        if (empty($segment->type)) {
            $failures[] = 'Missing/invalid type';
        }

        if ($segment->startAddress === null) {
            $failures[] = 'Missing start address';

        } elseif ($segment->startAddress > 0xFFFFFFFF) {
            $failures[] = 'Start address exceeds 32-bit unsigned integer';
        }

        if ($segment->size === null) {
            $failures[] = 'Missing size';

        } elseif ($segment->size > 0xFFFFFFFF) {
            $failures[] = 'Size exceeds 32-bit unsigned integer';
        }

        if ($segment->executable === null) {
            $failures[] = 'Missing executable';
        }

        $processedSectionKeys = [];
        foreach ($segment->sections as $section) {
            $failures = array_merge($failures, $this->validateMemorySegmentSection($section));

            if ($section->key !== null && in_array($section->key, $processedSectionKeys)) {
                $failures[] = 'Duplicate section key ("' . $section->key . '") detected';
            }

            foreach ($segment->sections as $sectionOther) {
                if ($section->key === $sectionOther->key) {
                    continue;
                }

                if ($section->intersectsWith($sectionOther)) {
                    $failures[] = 'Section "' . $section->key . '" overlaps with section "' . $sectionOther->key . '"';
                }
            }

            $processedSectionKeys[] = $section->key;
        }

        $totalSectionSize = $segment->totalSectionSize();
        if ($totalSectionSize > $segment->size) {
            $failures[] = 'Total size of all contained sections (' . $totalSectionSize . ' bytes) exceeds the total'
                . ' size of the memory segment (' . $segment->size . ' bytes)';
        }

        return array_map(
            fn (string $failure): string => 'Memory segment (key: "' . $segment->key . '") validation failure: '
                . $failure,
            $failures
        );
    }

    protected function validateMemorySegmentSection(MemorySegmentSection $section): array
    {
        $failures = [];

        if (empty($section->key)) {
            $failures[] = 'Missing key';
        }

        if (empty($section->name)) {
            $failures[] = 'Missing name';
        }

        if ($section->startAddress === null) {
            $failures[] = 'Missing start address';
        }

        if ($section->size === null) {
            $failures[] = 'Missing size';
        }

        $processedSectionKeys = [];
        foreach ($section->subSections as $subSection) {
            $failures = array_merge($failures, $this->validateMemorySegmentSection($subSection));

            if ($subSection->key !== null && in_array($subSection->key, $processedSectionKeys)) {
                $failures[] = 'Duplicate section key ("' . $subSection->key . '") detected';
            }

            foreach ($section->subSections as $sectionOther) {
                if ($subSection->key === $sectionOther->key) {
                    continue;
                }

                if ($subSection->intersectsWith($sectionOther)) {
                    $failures[] = 'Section "' . $subSection->key . '" overlaps with section "' . $sectionOther->key . '"';
                }
            }

            $processedSectionKeys[] = $subSection->key;
        }

        return array_map(
            fn (string $failure): string => 'Section (key: "' . $section->key . '") validation failure: ' . $failure,
            $failures
        );
    }

    protected function validateRegisterGroup(
        RegisterGroup $registerGroup,
        string $moduleKey,
        TargetDescriptionFile $tdf
    ): array
    {
        $failures = [];

        if (empty($registerGroup->key)) {
            $failures[] = 'Missing key';
        }

        if (!mb_check_encoding((string) $registerGroup->key, 'ASCII')) {
            $failures[] = 'Key contains non ASCII characters';
        }

        if (str_contains((string) $registerGroup->key, ' ')) {
            $failures[] = 'Key contains at least one white character';
        }

        if (str_contains((string) $registerGroup->key, '.')) {
            $failures[] = 'Key contains at least one period (".") character';
        }

        if (empty($registerGroup->name)) {
            $failures[] = 'Missing name';
        }

        if ($registerGroup->offset !== null && $registerGroup->offset > 0xFFFFFFFF) {
            $failures[] = 'Offset exceeds 32-bit unsigned integer';
        }

        $processedChildKeys = [];

        foreach ($registerGroup->registers as $register) {
            $failures = array_merge($failures, $this->validateRegister($register));

            if ($register->key !== null && in_array($register->key, $processedChildKeys)) {
                $failures[] = 'Duplicate register key ("' . $register->key . '") detected';
            }

            if ($register->alternative !== true) {
                foreach ($registerGroup->registers as $registerOther) {
                    if ($register->key === $registerOther->key || $registerOther->alternative === true) {
                        continue;
                    }

                    if ($register->intersectsWith($registerOther)) {
                        $failures[] = 'Register "' . $register->key . '" overlaps with register "'
                            . $registerOther->key . '"';
                    }
                }
            }

            $processedChildKeys[] = $register->key;
        }

        foreach ($registerGroup->subgroups as $subgroup) {
            $failures = array_merge(
                $failures,
                $this->validateRegisterGroup($subgroup, $moduleKey, $tdf)
            );

            if ($subgroup->key !== null && in_array($subgroup->key, $processedChildKeys)) {
                $failures[] = 'Duplicate register sub group key ("' . $subgroup->key . '") detected';
            }

            $processedChildKeys[] = $subgroup->key;
        }

        foreach ($registerGroup->subgroupReferences as $subgroupReference) {
            $failures = array_merge(
                $failures,
                $this->validateRegisterGroupReference($subgroupReference, $moduleKey, $tdf)
            );

            if ($subgroupReference->key !== null && in_array($subgroupReference->key, $processedChildKeys)) {
                $failures[] = 'Duplicate register group reference key ("' . $subgroupReference->key . '") detected';
            }

            $processedChildKeys[] = $subgroupReference->key;
        }

        return array_map(
            fn (string $failure): string => 'Register group (key: "' . $registerGroup->key . '") validation failure: '
                . $failure,
            $failures
        );
    }

    protected function validateRegisterGroupReference(
        RegisterGroupReference $registerGroupReference,
        string $moduleKey,
        TargetDescriptionFile $tdf
    ): array {
        $failures = [];

        if (empty($registerGroupReference->key)) {
            $failures[] = 'Missing key';
        }

        if (!mb_check_encoding((string) $registerGroupReference->key, 'ASCII')) {
            $failures[] = 'Key contains non ASCII characters';
        }

        if (str_contains((string) $registerGroupReference->key, ' ')) {
            $failures[] = 'Key contains at least one period space';
        }

        if (str_contains((string) $registerGroupReference->key, '.')) {
            $failures[] = 'Key contains at least one period (".") character';
        }

        if (empty($registerGroupReference->registerGroupKey)) {
            $failures[] = 'Missing register group key';
        }

        if (empty($registerGroupReference->name)) {
            $failures[] = 'Missing name';
        }

        if ($registerGroupReference->offset === null) {
            $failures[] = 'Missing offset';

        } elseif ($registerGroupReference->offset > 0xFFFFFFFF) {
            $failures[] = 'Offset exceeds 32-bit unsigned integer';
        }

        if ($tdf->resolveRegisterGroupReference($registerGroupReference, $moduleKey) === null) {
            $failures[] = 'Could not resolve register group reference "' . $registerGroupReference->key
                . '" - check register group key ("' . $registerGroupReference->registerGroupKey . '")';
        }

        return array_map(
            fn (string $failure): string => 'Register group reference (group key: "'
                . $registerGroupReference->registerGroupKey . '") validation failure: ' . $failure,
            $failures
        );
    }

    protected function validateRegister(Register $register): array
    {
        $failures = [];

        if (empty($register->key)) {
            $failures[] = 'Missing key';
        }

        if (!mb_check_encoding((string) $register->key, 'ASCII')) {
            $failures[] = 'Key contains non ASCII character';
        }

        if (str_contains((string) $register->key, ' ')) {
            $failures[] = 'Key contains at least white space character';
        }

        if (str_contains((string) $register->key, '.')) {
            $failures[] = 'Key contains at least one period (".") character';
        }

        if (empty($register->name)) {
            $failures[] = 'Missing name';
        }

        if ($register->offset === null) {
            $failures[] = 'Missing offset';

        } elseif ($register->offset > 0xFFFFFFFF) {
            $failures[] = 'Offset exceeds 32-bit unsigned integer';
        }

        if ($register->size === null) {
            $failures[] = 'Missing size';
        }

        $processedBitFieldKeys = [];
        foreach ($register->bitFields as $bitField) {
            $failures = array_merge($failures, $this->validateBitField($bitField));

            if ($bitField->key !== null && in_array($bitField->key, $processedBitFieldKeys)) {
                $failures[] = 'Duplicate bit field key ("' . $bitField->key . '") detected';
            }

            $processedBitFieldKeys[] = $bitField->key;
        }

        return array_map(
            fn (string $failure): string => 'Register (name: "' . $register->name . '") validation failure: '
                . $failure,
            $failures
        );
    }

    protected function validateBitField(BitField $bitField): array
    {
        $failures = [];

        if (empty($bitField->key)) {
            $failures[] = 'Missing key';
        }

        if (empty($bitField->name)) {
            $failures[] = 'Missing name';
        }

        if ($bitField->mask === null) {
            $failures[] = 'Missing mask';
        }

        return array_map(
            fn (string $failure): string => 'Bit field (name: "' . $bitField->name . '") validation failure: '
                . $failure,
            $failures
        );
    }

    protected function validateModule(Module $module, TargetDescriptionFile $tdf): array
    {
        $failures = [];

        if (empty($module->key)) {
            $failures[] = 'Missing key';
        }

        if (empty($module->name)) {
            $failures[] = 'Missing name';
        }

        if (empty($module->description)) {
            $failures[] = 'Missing description';
        }

        $processedChildKeys = [];
        foreach ($module->registerGroups as $registerGroup) {
            $failures = array_merge(
                $failures,
                $this->validateRegisterGroup($registerGroup, $module->key ?? '', $tdf)
            );

            if ($registerGroup->key !== null && in_array($registerGroup->key, $processedChildKeys)) {
                $failures[] = 'Duplicate register group key ("' . $registerGroup->key . '") detected';
            }

            $processedChildKeys[] = $registerGroup->key;
        }

        return array_map(
            fn (string $failure): string => 'Module (key: "' . $module->key . '") validation failure: ' . $failure,
            $failures
        );
    }

    protected function validatePeripheral(Peripheral $peripheral, TargetDescriptionFile $tdf): array
    {
        $failures = [];

        if (empty($peripheral->key)) {
            $failures[] = 'Missing key';
        }

        if (empty($peripheral->name)) {
            $failures[] = 'Missing name';
        }

        if (empty($peripheral->moduleKey)) {
            $failures[] = 'Missing module key';
        }

        if (empty($peripheral->registerGroupInstances) && empty($peripheral->signals)) {
            $failures[] = 'Empty - no register group instances or signals';
        }

        $processedChildKeys = [];
        foreach ($peripheral->registerGroupInstances as $registerGroupInstance) {
            $failures = array_merge(
                $failures,
                $this->validateRegisterGroupInstance(
                    $registerGroupInstance,
                    $peripheral->moduleKey ?? '',
                    $tdf
                )
            );

            if ($registerGroupInstance->key !== null && in_array($registerGroupInstance->key, $processedChildKeys)) {
                $failures[] = 'Duplicate register group instance key ("' . $registerGroupInstance->key . '") detected';
            }

            $processedChildKeys[] = $registerGroupInstance->key;
        }

        foreach ($peripheral->signals as $signal) {
            $failures = array_merge($failures, $this->validateSignal($signal));
        }

        return array_map(
            fn (string $failure): string => 'Peripheral "' . $peripheral->name . '" validation failure: ' . $failure,
            $failures
        );
    }

    protected function validateRegisterGroupInstance(
        RegisterGroupInstance $registerGroupInstance,
        string $moduleKey,
        TargetDescriptionFile $tdf
    ): array {
        $failures = [];

        if (empty($registerGroupInstance->key)) {
            $failures[] = 'Missing key';
        }

        if (!mb_check_encoding((string) $registerGroupInstance->key, 'ASCII')) {
            $failures[] = 'Key contains non ASCII characters';
        }

        if (str_contains((string) $registerGroupInstance->key, ' ')) {
            $failures[] = 'Key contains at least one period space';
        }

        if (str_contains((string) $registerGroupInstance->key, '.')) {
            $failures[] = 'Key contains at least one period (".") character';
        }

        if (empty($registerGroupInstance->registerGroupKey)) {
            $failures[] = 'Missing register group key';
        }

        if (empty($registerGroupInstance->name)) {
            $failures[] = 'Missing name';
        }

        if ($registerGroupInstance->offset === null) {
            $failures[] = 'Missing offset';

        } elseif ($registerGroupInstance->offset > 0xFFFFFFFF) {
            $failures[] = 'Offset exceeds 32-bit unsigned integer';
        }

        if (empty($registerGroupInstance->addressSpaceKey)) {
            $failures[] = 'Missing address space key';

        } elseif ($tdf->getAddressSpace($registerGroupInstance->addressSpaceKey) === null) {
            $failures[] = 'Could not find address space "' . $registerGroupInstance->addressSpaceKey
                . '" - check address space key';
        }

        if ($tdf->resolveRegisterGroupInstance($registerGroupInstance, $moduleKey) === null) {
            $failures[] = 'Could not resolve register group instance "' . $registerGroupInstance->key
                . '" - check register group key ("' . $registerGroupInstance->registerGroupKey . '")';
        }

        return array_map(
            fn (string $failure): string => 'Register group instance (group key: "'
                . $registerGroupInstance->registerGroupKey . '") validation failure: ' . $failure,
            $failures
        );
    }

    protected function validateSignal(Signal $signal): array
    {
        $failures = [];

        if (empty($signal->padId)) {
            $failures[] = 'Missing pad ID';
        }

        return array_map(
            fn (string $failure): string => 'Signal validation failure: ' . $failure,
            $failures
        );
    }

    protected function validatePinout(Pinout $pinout): array
    {
        $failures = [];

        if (empty($pinout->key)) {
            $failures[] = 'Missing key';
        }

        if (empty($pinout->name)) {
            $failures[] = 'Missing name';
        }

        if (empty($pinout->type)) {
            $failures[] = 'Unknown type';
        }

        if (
            in_array($pinout->type, [PinoutType::SOIC, PinoutType::DIP, PinoutType::SSOP])
            && count($pinout->pins) % 2 != 0
        ) {
            $failures[] = 'Pin count for ' . $pinout->type->value . ' pinout is not a multiple of two';
        }

        if (
            in_array($pinout->type, [PinoutType::QFP, PinoutType::QFN])
            && count($pinout->pins) % 2 != 0
        ) {
            $failures[] = 'Pin count for ' . $pinout->type->value . ' pinout is not a multiple of four';
        }

        // It's common for the pin count to be included in the name - we verify it here
        $impliedPinCount = filter_var($pinout->name, FILTER_SANITIZE_NUMBER_INT);
        if (is_numeric($impliedPinCount) && (int) $impliedPinCount !== count($pinout->pins)) {
            $failures[] = 'Implied pin count (' . $impliedPinCount . ') does not match actual pin count ('
                . count($pinout->pins) . ')';
        }

        $processedPositions = [];
        foreach ($pinout->pins as $pin) {
            $failures = array_merge($failures, $this->validatePin($pin, $pinout));

            if ($pin->position !== null && in_array($pin->position, $processedPositions)) {
                $failures[] = 'Duplicate pin position (' . $pin->position . ') detected';
            }

            $processedPositions[] = $pin->position;
        }

        return array_map(
            fn (string $failure): string => 'Pinout "' . $pinout->key . '" validation failure: ' . $failure,
            $failures
        );
    }

    protected function validatePin(Pin $pin, Pinout $pinout): array
    {
        $failures = [];

        if (empty($pin->position)) {
            $failures[] = 'Missing position';

        } elseif (
            in_array(
                $pinout->type,
                [PinoutType::QFN, PinoutType::QFP, PinoutType::MLF, PinoutType::DIP, PinoutType::SOIC]
            )
            && !is_numeric($pin->position)
        ) {
            $failures[] = 'Position for ' . $pinout->type->value . ' pinouts must be numerical (current value: "'
                . $pin->position . '")';
        }

        if (empty($pin->pad)) {
            $failures[] = 'Missing pad';
        }

        return array_map(
            fn (string $failure): string => 'Pin validation failure: ' . $failure,
            $failures
        );
    }

    protected function validateVariant(Variant $variant, TargetDescriptionFile $tdf): array
    {
        $failures = [];

        if (empty($variant->name)) {
            $failures[] = 'Missing name';
        }

        if (empty($variant->pinoutKey)) {
            $failures[] = 'Missing pinout key';

        } elseif (!$tdf->getPinout($variant->pinoutKey) instanceof Pinout) {
            $failures[] = 'Could not resolve pinout from key "' . $variant->pinoutKey
                . '" - check pinout key';
        }

        if (empty($variant->package)) {
            $failures[] = 'Missing package';
        }

        return array_map(
            fn (string $failure): string => 'Variant "' . $variant->name . '" validation failure: ' . $failure,
            $failures
        );
    }
}
