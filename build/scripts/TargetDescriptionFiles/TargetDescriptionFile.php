<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

use Exception;
use SimpleXMLElement;

require_once __DIR__ . "/Variant.php";
require_once __DIR__ . "/AddressSpace.php";
require_once __DIR__ . "/PropertyGroup.php";
require_once __DIR__ . "/Module.php";
require_once __DIR__ . "/PhysicalInterface.php";

class TargetDescriptionFile
{
    const ARCHITECTURE_AVR8 = 'AVR8';

    /** @var AddressSpace[] */
    protected array $addressSpacesById = [];

    /** @var PropertyGroup[] */
    protected array $propertyGroupsByName = [];

    /** @var Module[] */
    protected array $modulesByName = [];

    /** @var Module[] */
    protected array $peripheralModulesByName = [];

    /** @var PhysicalInterface[] */
    protected array $physicalInterfacesByName = [];

    public string $filePath;
    public ?SimpleXMLElement $xml = null;

    public ?string $targetName = null;
    public ?string $targetArchitecture = null;

    /** @var Variant[] */
    public array $variants = [];

    public function __construct(string $filePath)
    {
        $this->filePath = $filePath;
        $this->init();
    }

    protected function init()
    {
        if (!file_exists($this->filePath)) {
            throw new Exception("Invalid TDF file path - file does not exist.");
        }

        $xml = simplexml_load_file($this->filePath);
        if ($xml === false) {
            throw new Exception("Failed to parse TDF XML.");
        }

        $this->xml = $xml;

        $device = $this->xml->device;
        if (!empty($device)) {
            $deviceAttributes = $device->attributes();

            if (!empty($deviceAttributes['name'])) {
                $this->targetName = $device['name'];
            }

            if (!empty($deviceAttributes['architecture'])) {
                $this->targetArchitecture = stristr($device['architecture'], 'avr') !== false
                    ? self::ARCHITECTURE_AVR8 : $device['architecture'];
            }
        }

        $this->loadVariants();
        $this->loadAddressSpaces();
        $this->loadPropertyGroups();
        $this->loadModules();
        $this->loadPeripheralModules();
        $this->loadPhysicalInterfaces();
    }

    protected function rawValueToInt(string $value): ?int
    {
        return stristr($value, '0x') !== false ? (int) hexdec($value) :
            (strlen($value) > 0 ? (int) $value : null);
    }

    private function loadVariants(): void
    {
        $variantElements = $this->xml->xpath('variants/variant');
        foreach ($variantElements as $variantElement) {
            $variantAttributes = $variantElement->attributes();
            $variant = new Variant();

            if (!empty($variantAttributes['ordercode'])) {
                $variant->name = $variantAttributes['ordercode'];
            }

            if (!empty($variantAttributes['name'])) {
                $variant->name = $variantAttributes['name'];
            }

            if (!empty($variantAttributes['package'])) {
                $variant->package = $variantAttributes['package'];
            }

            if (!empty($variantAttributes['pinout'])) {
                $variant->pinout = $variantAttributes['pinout'];
            }

            $this->variants[] = $variant;
        }
    }

    private function loadAddressSpaces(): void
    {
        $addressSpaceElements = $this->xml->xpath('device/address-spaces/address-space');
        foreach ($addressSpaceElements as $addressSpaceElement) {
            $addressSpaceAttrs = $addressSpaceElement->attributes();
            $addressSpace = new AddressSpace();

            $addressSpace->id = isset($addressSpaceElement['id']) ? $addressSpaceElement['id'] : null;
            if (is_null($addressSpace->id)) {
                // All address spaces must have an ID - don't bother if one isn't found
                continue;
            }

            $addressSpace->name = isset($addressSpaceAttrs['name']) ? $addressSpaceAttrs['name'] : null;
            $addressSpace->startAddress = isset($addressSpaceAttrs['start'])
                ? $this->rawValueToInt($addressSpaceAttrs['start']) : null;
            $addressSpace->size = isset($addressSpaceAttrs['size'])
                ? $this->rawValueToInt($addressSpaceAttrs['size']) : null;

            $memorySegmentElements = $addressSpaceElement->xpath('memory-segment');
            foreach ($memorySegmentElements as $memorySegmentElement) {
                $memorySegmentAttrs = $memorySegmentElement->attributes();
                $memorySegment = new MemorySegment();

                $memorySegment->name = isset($memorySegmentAttrs['name']) ? $memorySegmentAttrs['name'] : null;
                $memorySegment->startAddress = isset($memorySegmentAttrs['start'])
                    ? $this->rawValueToInt($memorySegmentAttrs['start']) : null;
                $memorySegment->type = isset($memorySegmentAttrs['type']) ? $memorySegmentAttrs['type'] : null;
                $memorySegment->size = isset($memorySegmentAttrs['size'])
                    ? $this->rawValueToInt($memorySegmentAttrs['size']) : null;
                $memorySegment->pageSize = isset($memorySegmentAttrs['pagesize'])
                    ? $this->rawValueToInt($memorySegmentAttrs['pagesize']) : null;

                $addressSpace->memorySegmentsByTypeAndName[strtolower($memorySegment->type)]
                [strtolower($memorySegment->name)] = $memorySegment;
            }

            $this->addressSpacesById[strtolower($addressSpace->id)] = $addressSpace;
        }
    }

    private function loadPropertyGroups(): void
    {
        $propertyGroupElements = $this->xml->xpath('device/property-groups/property-group');
        foreach ($propertyGroupElements as $propertyGroupElement) {
            $propertyGroupAttrs = $propertyGroupElement->attributes();
            $propertyGroup = new PropertyGroup();

            $propertyGroup->name = isset($propertyGroupAttrs['name']) ? $propertyGroupAttrs['name'] : null;
            if (empty($propertyGroup->name)) {
                continue;
            }

            $propertyElements = $propertyGroupElement->xpath('property');
            foreach ($propertyElements as $propertyElement) {
                $propertyAttrs = $propertyElement->attributes();
                $property = new Property();

                $property->name = isset($propertyAttrs['name']) ? $propertyAttrs['name'] : null;
                if (empty($propertyGroup->name)) {
                    continue;
                }

                $property->value = isset($propertyAttrs['value']) ? $propertyAttrs['value'] : null;

                $propertyGroup->propertiesMappedByName[strtolower($property->name)] = $property;
            }

            $this->propertyGroupsByName[strtolower($propertyGroup->name)] = $propertyGroup;
        }
    }

    private function generateRegisterGroupFromElement(SimpleXMLElement $registerGroupElement): ?RegisterGroup
    {
        $registerGroupAttrs = $registerGroupElement->attributes();
        $registerGroup = new RegisterGroup();

        $registerGroup->name = isset($registerGroupAttrs['name']) ? $registerGroupAttrs['name'] : null;
        if (empty($registerGroup->name)) {
            return null;
        }

        $registerGroup->offset = isset($registerGroupAttrs['offset'])
            ? $this->rawValueToInt($registerGroupAttrs['offset']) : null;

        $registerElements = $registerGroupElement->xpath('register');
        foreach ($registerElements as $registerElement) {
            $registerAttrs = $registerElement->attributes();
            $register = new Register();

            $register->name = isset($registerAttrs['name']) ? $registerAttrs['name'] : null;
            if (empty($register->name)) {
                continue;
            }

            $register->offset = isset($registerAttrs['offset'])
                ? $this->rawValueToInt($registerAttrs['offset']) : null;
            $register->size = isset($registerAttrs['size'])
                ? $this->rawValueToInt($registerAttrs['size']) : null;

            $registerGroup->registersMappedByName[strtolower($register->name)] = $register;
        }

        return $registerGroup;
    }

    private function generateModuleFromElement(SimpleXMLElement $moduleElement): ?Module
    {
        $moduleAttrs = $moduleElement->attributes();
        $module = new Module();

        $module->name = isset($moduleAttrs['name']) ? $moduleAttrs['name'] : null;
        if (empty($module->name)) {
            return null;
        }

        $registerGroupElements = $moduleElement->xpath('register-group');
        foreach ($registerGroupElements as $registerGroupElement) {
            $registerGroup = $this->generateRegisterGroupFromElement($registerGroupElement);

            if ($registerGroup instanceof RegisterGroup) {
                $module->registerGroupsMappedByName[strtolower($registerGroup->name)] = $registerGroup;
            }
        }

        $instanceElements = $moduleElement->xpath('instance');
        foreach ($instanceElements as $instanceElement) {
            $instanceAttrs = $instanceElement->attributes();
            $moduleInstance = new ModuleInstance();

            $moduleInstance->name = isset($instanceAttrs['name']) ? $instanceAttrs['name'] : null;
            if (empty($moduleInstance->name)) {
                continue;
            }

            $registerGroupElements = $instanceElement->xpath('register-group');
            foreach ($registerGroupElements as $registerGroupElement) {
                $registerGroup = $this->generateRegisterGroupFromElement($registerGroupElement);

                if ($registerGroup instanceof RegisterGroup) {
                    $moduleInstance->registerGroupsMappedByName[strtolower($registerGroup->name)] = $registerGroup;
                }
            }

            $module->instancesMappedByName[strtolower($moduleInstance->name)] = $moduleInstance;
        }

        return $module;
    }

    private function loadModules(): void
    {
        $moduleElements = $this->xml->xpath('modules/module');
        foreach ($moduleElements as $moduleElement) {
            $module = $this->generateModuleFromElement($moduleElement);

            if ($module instanceof Module) {
                $this->modulesByName[strtolower($module->name)] = $module;
            }
        }
    }

    private function loadPeripheralModules(): void
    {
        $moduleElements = $this->xml->xpath('device/peripherals/module');
        foreach ($moduleElements as $moduleElement) {
            $module = $this->generateModuleFromElement($moduleElement);

            if ($module instanceof Module) {
                $this->peripheralModulesByName[strtolower($module->name)] = $module;
            }
        }
    }

    private function loadPhysicalInterfaces(): void
    {
        $interfaceElements = $this->xml->xpath('device/interfaces/interface');
        foreach ($interfaceElements as $interfaceElement) {
            $interfaceAttrs = $interfaceElement->attributes();
            $physicalInterface = new PhysicalInterface();

            $physicalInterface->name = isset($interfaceAttrs['name']) ? $interfaceAttrs['name'] : null;
            if (empty($physicalInterface->name)) {
                continue;
            }

            $physicalInterface->type = isset($interfaceAttrs['type']) ? $interfaceAttrs['type'] : null;

            $this->physicalInterfacesByName[strtolower($physicalInterface->name)] = $physicalInterface;
        }
    }

    public function validate(): array
    {
        $failures = [];

        if (empty($this->targetName)) {
            $failures[] = 'Target name not found';
        }

        if (empty($this->targetArchitecture)) {
            $failures[] = 'Target architecture not found';
        }

        if (empty($this->variants)) {
            $failures[] = 'Missing target variants';
        }

        foreach ($this->variants as $variant) {
            $variantValidationFailures = $variant->validate();

            if (!empty($variantValidationFailures)) {
                $failures[] = 'Variant validation failures: ' . implode(", ", $variantValidationFailures);
            }
        }

        return $failures;
    }
}
