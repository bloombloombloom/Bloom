<?php
namespace Targets\TargetDescriptionFiles\Services\Xml;

use DOMDocument;
use DOMNode;
use DOMElement;
use DOMNodeList;
use RuntimeException;
use Targets\TargetDescriptionFiles\AVR8\Services\ValidationService;
use Targets\TargetDescriptionFiles\Pad;
use Targets\TargetDescriptionFiles\Services\StringService;
use Targets\TargetDescriptionFiles\Avr8\Avr8TargetDescriptionFile;
use Targets\TargetDescriptionFiles\Avr8\AvrFamily;
use Targets\TargetDescriptionFiles\TargetFamily;
use Targets\TargetDescriptionFiles\PropertyGroup;
use Targets\TargetDescriptionFiles\Property;
use Targets\TargetDescriptionFiles\AddressSpace;
use Targets\TargetDescriptionFiles\PhysicalInterface;
use Targets\TargetDescriptionFiles\MemorySegment;
use Targets\TargetDescriptionFiles\MemorySegmentType;
use Targets\TargetDescriptionFiles\MemorySegmentSection;
use Targets\TargetDescriptionFiles\Peripheral;
use Targets\TargetDescriptionFiles\RegisterGroupInstance;
use Targets\TargetDescriptionFiles\Signal;
use Targets\TargetDescriptionFiles\Module;
use Targets\TargetDescriptionFiles\RegisterGroup;
use Targets\TargetDescriptionFiles\RegisterGroupReference;
use Targets\TargetDescriptionFiles\Register;
use Targets\TargetDescriptionFiles\BitField;
use Targets\TargetDescriptionFiles\Pinout;
use Targets\TargetDescriptionFiles\PinoutType;
use Targets\TargetDescriptionFiles\Pin;
use Targets\TargetDescriptionFiles\Variant;
use Targets\TargetPhysicalInterface;

require_once __DIR__ . '/StringService.php';
require_once __DIR__ . '/../AVR8/Avr8TargetDescriptionFile.php';

/**
 * This AtdfService provides ATDF -> TDF conversion.
 *
 * We use this to convert ATDFs of new Microchip AVR targets to Bloom's TDF format.
 * See the ConvertAtdfToTdf.php script for more.
 *
 * NOTE: This service class is only intended for performing a first-pass conversion - it will do most of the work, but
 * may not cover every corner, so the generated TDF may require some manual touch-ups. It's best to run the generated
 * TDF through validation (@see ValidationService;), to identify any issues that require attention.
 */
class AtdfService
{
    private StringService $stringService;

    public function __construct(?StringService $stringService = null)
    {
        $this->stringService = $stringService ?? new StringService();
    }

    /**
     * Searches for ATDF files in the given directory, recursively.
     *
     * Rudimentary implementation - filters files by .atdf file extension.
     *
     * @param string $directoryPath
     *
     * @return \SplFileInfo[]
     *  An SplFileInfo instance for each ATDF file.
     */
    public function findFiles(string $directoryPath): array
    {
        $output = [];

        $directory = new \DirectoryIterator($directoryPath);
        foreach ($directory as $entry) {
            if ($entry->isFile() && strtolower($entry->getExtension()) === 'atdf') {
                $output[] = clone $entry;

            } elseif ($entry->isDir() && !$entry->isDot()) {
                $output = array_merge($output, $this->findFiles($entry->getPathname()));
            }
        }

        return $output;
    }

    /**
     * Processes an ATDF XML document and generates a TDF object.
     *
     * @param DOMDocument $document
     *  The ATDF XML document to process.
     *
     * @return Avr8TargetDescriptionFile
     *  The generated TDF object.
     *
     * @throws \Exception
     */
    public function toTdf(DOMDocument $document): Avr8TargetDescriptionFile
    {
        $tdf = new Avr8TargetDescriptionFile();

        $deviceElements = $this->getElementsFromXPath('devices/device', $document);
        if ($deviceElements->count() !== 1) {
            throw new RuntimeException('Unexpected number of device elements');
        }
        $deviceElement = $deviceElements->item(0);

        $deviceAttributesByName = $this->getNodeAttributesByName($deviceElement);
        $tdf->deviceAttributesByName = [
            'name' => $deviceAttributesByName['name'] ?? null,
            'family' => TargetFamily::AVR_8->value,
            'architecture' => $deviceAttributesByName['architecture'] ?? null,
            'configuration-value' => isset($deviceAttributesByName['name'])
                ? strtolower($deviceAttributesByName['name'])
                : null,
            'avr-family' => $this->resolveAvrFamily(
                $deviceAttributesByName['family'] ?? '',
                $deviceAttributesByName['name'] ?? ''
            )->value ?? null,
        ];

        $propertyGroupElements = $this->getDeviceElementsFromXPath(
            'property-groups/property-group',
            $document
        );
        foreach ($propertyGroupElements as $element) {
            $tdf->propertyGroups[] = $this->propertyGroupFromElement($element);
        }

        $addressSpaceElements = $this->getDeviceElementsFromXPath(
            'address-spaces/address-space',
            $document
        );
        foreach ($addressSpaceElements as $element) {
            $tdf->addressSpaces[] = $this->addressSpaceFromElement($element, $tdf);
        }

        $interfaceElements = $this->getDeviceElementsFromXPath(
            'interfaces/interface',
            $document
        );
        foreach ($interfaceElements as $element) {
            $tdf->physicalInterfaces[] = $this->physicalInterfaceFromElement($element);
        }

        $peripheralModuleElements = $this->getDeviceElementsFromXPath(
            'peripherals/module',
            $document
        );
        foreach ($peripheralModuleElements as $peripheralModuleElement) {
            foreach ($peripheralModuleElement->getElementsByTagName('instance') as $element) {
                $tdf->peripherals[] = $this->peripheralFromElement(
                    $element,
                    $this->getNodeAttributesByName($peripheralModuleElement)
                );
            }
        }

        $moduleElements = $this->getElementsFromXPath(
            'modules/module',
            $document
        );
        foreach ($moduleElements as $element) {
            $tdf->modules[] = $this->moduleFromElement($element);
        }

        $padsByKey = [];
        $pinoutElements = $this->getElementsFromXPath(
            'pinouts/pinout',
            $document
        );
        foreach ($pinoutElements as $element) {
            $tdf->pinouts[] = $this->pinoutFromElement($element);

            foreach ($element->childNodes as $childNode) {
                if (!$childNode instanceof DOMElement) {
                    continue;
                }

                if ($childNode->nodeName === 'pin') {
                    $pad = $this->padFromPinElement($childNode);
                    if ($pad->key === null || $pad->key === 'nc') {
                        continue;
                    }

                    $padsByKey[$pad->key] = $pad;
                }
            }
        }

        $tdf->pads = array_values($padsByKey);

        // Sort pads by key
        uasort($tdf->pads, function (Pad $padA, Pad $padB): bool {
            return $padA->key > $padB->key;
        });

        $variantElements = $this->getElementsFromXPath(
            'variants/variant',
            $document
        );
        foreach ($variantElements as $element) {
            $tdf->variants[] = $this->variantFromElement($element);
        }

        $moduleKeysToRenameByName = [
            'port' => 'gpio_port'
        ];

        foreach ($tdf->modules as $module) {
            if (array_key_exists(strtolower($module->name), $moduleKeysToRenameByName)) {
                $tdf->renameModuleKey($module, $moduleKeysToRenameByName[strtolower($module->name)]);
            }
        }

        // Rename GPIO port signals
        foreach ($tdf->peripherals as $peripheral) {
            if ($peripheral->moduleKey !== 'gpio_port') {
                continue;
            }

            foreach ($peripheral->signals as $signal) {
                $signal->name = strtoupper((string) $signal->padKey);
            }
        }

        if (in_array(TargetPhysicalInterface::UPDI, $tdf->getSupportedDebugPhysicalInterfaces())) {
            /*
             * ATDFs for UPDI-enabled targets do not typically possess an `ocd_base_addr` property in the updi_interface
             * property group. Bloom needs this property to configure EDBG debug tools, so we add it here.
             *
             * The value of the property (0x00000F80) seems to be the same across all UPDI-enabled targets, so maybe
             * that's why it wasn't included in the ATDFs.
             */
            if (
                ($updiInterfacePropertyGroup = $tdf->getPropertyGroup('updi_interface')) !== null
                && $updiInterfacePropertyGroup->getProperty('ocd_base_addr') === null
            ) {
                $updiInterfacePropertyGroup->properties[] = new Property('ocd_base_addr', '0x00000F80');
            }
        }

        if ($tdf->getAvrFamily() === AvrFamily::XMEGA) {
            /*
             * ATDFs for XMEGA targets do not typically possess a `signature_offset` property in the pdi_interface
             * property group. Bloom needs this property to configure EDBG debug tools, so we add it here.
             *
             * The value of the property (0x00000090) seems to be the same across all XMEGA targets, so maybe that's
             * why it wasn't included in the ATDFs.
             */
            if (
                ($pdiInterfacePropertyGroup = $tdf->getPropertyGroup('pdi_interface')) !== null
                && $pdiInterfacePropertyGroup->getProperty('signature_offset') === null
            ) {
                $pdiInterfacePropertyGroup->properties[] = new Property('signature_offset', '0x00000090');
            }
        }

        return $tdf;
    }

    private function getElementsFromXPath(string $expression, DOMDocument $document): DOMNodeList
    {
        $elements = (new \DOMXPath($document))->query('/avr-tools-device-file/' . $expression);

        if ($elements === false) {
            throw new \Exception('Failed to evaluate XPath expression - malformed expression');
        }

        return $elements;
    }

    private function getDeviceElementsFromXPath(string $expression, DOMDocument $document): DOMNodeList
    {
        return $this->getElementsFromXPath('devices/device[1]/' . $expression, $document);
    }

    private function getNodeAttributesByName(DOMNode $node): array
    {
        $output = [];
        foreach ($node->attributes as $attribute) {
            /** @var \DOMAttr $attribute */
            $output[$attribute->name] = $attribute->value;
        }

        return $output;
    }

    private function resolveAvrFamily(string $atdfFamilyName, string $atdfTargetName): ?AvrFamily
    {
        $atdfFamilyName = strtolower(trim($atdfFamilyName));
        $atdfTargetName = strtolower(trim($atdfTargetName));

        if (str_contains($atdfFamilyName, 'xmega') !== false) {
            return AvrFamily::XMEGA;
        }

        if (str_contains($atdfFamilyName, 'tiny') !== false) {
            return AvrFamily::TINY;
        }

        if (str_contains($atdfFamilyName, 'mega') !== false) {
            return AvrFamily::MEGA;
        }

        if (strlen($atdfTargetName) >= 7 && str_starts_with($atdfTargetName, 'avr')) {
            if ($atdfFamilyName === 'avr da' || substr($atdfTargetName, 5, 2) === 'da') {
                return AvrFamily::DA;
            }

            if ($atdfFamilyName === 'avr db' || substr($atdfTargetName, 5, 2) === 'db') {
                return AvrFamily::DB;
            }

            if ($atdfFamilyName === 'avr dd' || substr($atdfTargetName, 5, 2) === 'dd') {
                return AvrFamily::DD;
            }

            if ($atdfFamilyName === 'avr ea' || substr($atdfTargetName, 5, 2) === 'ea') {
                return AvrFamily::EA;
            }
        }

        return null;
    }

    private function propertyGroupFromElement(DOMElement $element): PropertyGroup
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new PropertyGroup(isset($attributes['name']) ? strtolower($attributes['name']) : null, [], []);

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'property') {
                $output->properties[] = $this->propertyFromElement($childNode);
            }
        }

        return $output;
    }

    private function propertyFromElement(DOMElement $element): Property
    {
        $attributes = $this->getNodeAttributesByName($element);
        return new Property(
            isset($attributes['name']) ? strtolower($attributes['name']) : null,
            $attributes['value'] ?? null,
        );
    }

    private function addressSpaceFromElement(DOMElement $element, Avr8TargetDescriptionFile $tdf): AddressSpace
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new AddressSpace(
            isset($attributes['id']) ? strtolower($attributes['id']) : null,
            $this->stringService->tryStringToInt($attributes['start'] ?? null),
            $this->stringService->tryStringToInt($attributes['size'] ?? null),
            null,
            $attributes['endianness'] ?? null,
        );

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'memory-segment') {
                $output->memorySegments[] = $this->memorySegmentFromElement($childNode);
            }
        }

        if ($tdf->getAvrFamily() === AvrFamily::MEGA && $output->key === 'prog') {
            /*
             * ATDFs for some MEGA targets contain boot section **options** as memory segments in the `prog` address
             * space. By "options", I mean the target can be configured with one of these boot sections, via the
             * setting of some fuse/register.
             *
             * So these are not actual memory segments - they're just potential sections of the program memory segment.
             * For this reason, we convert them into property groups and discard the segments.
             */
            $bootSectionPropertyGroups = [];
            $filteredSegments = [];

            foreach ($output->memorySegments as $segment) {
                if (!str_starts_with($segment->key, 'boot_section_')) {
                    $filteredSegments[] = $segment;
                    continue;
                }

                $bootSectionPropertyGroups[] = new PropertyGroup(
                    $segment->key,
                    [],
                    [
                        new Property(
                            'start_address',
                            '0x' . str_pad(
                                strtoupper(dechex($segment->startAddress)),
                                8,
                                '0',
                                STR_PAD_LEFT
                            )
                        ),
                        new Property('size', $segment->size),
                        new Property('page_size', $segment->pageSize),
                    ]
                );
            }

            if (!empty($bootSectionPropertyGroups)) {
                $tdf->propertyGroups[] = new PropertyGroup(
                    'boot_section_options',
                    $bootSectionPropertyGroups,
                    []
                );

                $output->memorySegments = $filteredSegments;
            }
        }

        /*
         * Bloom will look up certain memory segments using specific keys, so we must ensure that these segments hold
         * the correct key.
         *
         * While we're at it, we also rename some other keys and segment names, for the sake of consistency.
         */
        $segmentKeysToRename = [
            'flash' => 'internal_program_memory',
            'progmem' => 'internal_program_memory',
            'internal_sram' => 'internal_ram',
            'iram' => 'internal_ram',
            'external_sram' => 'external_ram',
            'eeprom' => 'internal_eeprom',
            'registers' => 'gp_registers',
        ];

        $segmentNamesToRename = [
            'boot_section' => 'Boot Section',
            'io' => 'Input/Output',
            'mapped_io' => 'Mapped Input/Output',
            'mapped_eeprom' => 'Mapped EEPROM',
            'internal_ram' => 'Internal RAM',
            'external_ram' => 'External RAM',
            'internal_program_memory' => 'Internal FLASH',
            'internal_eeprom' => 'Internal EEPROM',
            'gp_registers' => 'General Purpose Registers',
            'signatures' => 'Signatures',
            'fuses' => 'Fuses',
            'lockbits' => 'Lockbits',
            'user_signatures' => 'User Signatures',
            'prod_signatures' => 'Production Signatures',
        ];

        foreach ($output->memorySegments as $segment) {
            if (array_key_exists($segment->key, $segmentKeysToRename)) {
                $segment->key = $segmentKeysToRename[$segment->key];
            }

            if (array_key_exists($segment->key, $segmentNamesToRename)) {
                $segment->name = $segmentNamesToRename[$segment->key];
            }
        }

        /*
         * Like Bloom's TDFs, ATDFs have sections within memory segments, but ATDFs do not have a specific element for
         * sections. They just use overlapping `<memory-segment>` elements to describe sections.
         *
         * Bloom's TDFs have a dedicated element for sections, which reside in memory segment elements and can be
         * nested.
         *
         * So now, we must convert all overlapping memory segments to sections.
         */

        $convertedSegmentKeys = [];

        foreach ($output->memorySegments as $segment) {
            if (in_array($segment->key, $convertedSegmentKeys)) {
                continue;
            }

            foreach ($output->memorySegments as $otherSegment) {
                if ($otherSegment->key === $segment->key) {
                    continue;
                }

                if ($segment->contains($otherSegment)) {
                    $convertedSegmentKeys[] = $otherSegment->key;

                    $section = new MemorySegmentSection(
                        $otherSegment->key,
                        $otherSegment->name,
                        $otherSegment->startAddress,
                        $otherSegment->size,
                        []
                    );

                    /*
                     * Find the appropriate parent section, if there is one. Otherwise, just store the new section in
                     * the segment.
                     */
                    if (
                        $otherSegment->startAddress !== null
                        && $otherSegment->size !== null
                        && ($parentSection = $segment->getInnermostSectionContainingAddressRange(
                            $otherSegment->startAddress,
                            ($otherSegment->startAddress + $otherSegment->size - 1)
                        )) !== null
                    ) {
                        $parentSection->subSections[] = $section;

                    } else {
                        $segment->sections[] = $section;
                    }
                }
            }
        }

        // Remove the converted memory segments
        $output->memorySegments = array_filter(
            $output->memorySegments,
            fn (MemorySegment $segment): bool => !in_array($segment->key, $convertedSegmentKeys)
        );

        /*
         * ATDFs for XMEGA targets have a separate memory segment for the application and boot section of program
         * memory, but they have no segment for the entire program memory. This is inconsistent with the other ATDFs
         * and Bloom expects all TDFs to contain one internal_program_memory segment.
         *
         * For these reasons, we overwrite the program memory address space with a better structured memory segment.
         */
        if (
            $output->key === 'prog'
            && $tdf->getAvrFamily() === AvrFamily::XMEGA
            && ($appSectionSegment = $output->getMemorySegment('app_section')) !== null
            && ($bootSectionSegment = $output->getMemorySegment('boot_section')) !== null
            && ($appTableSection = $appSectionSegment->getSection('apptable_section')) !== null
        ) {
            $output->memorySegments = [
                new MemorySegment(
                    'internal_program_memory',
                    'Internal FLASH',
                    MemorySegmentType::FLASH,
                    $appSectionSegment->startAddress,
                    $appSectionSegment->size + $bootSectionSegment->size,
                    $appSectionSegment->pageSize,
                    $appSectionSegment->access,
                    [
                        new MemorySegmentSection(
                            'app_section',
                            'Application Section',
                            $appSectionSegment->startAddress,
                            $appSectionSegment->size,
                            [
                                new MemorySegmentSection(
                                    'app_table_section',
                                    'Application Table Section',
                                    $appTableSection->startAddress,
                                    $appTableSection->size,
                                    []
                                )
                            ]
                        ),
                        new MemorySegmentSection(
                            'boot_section',
                            'Boot Loader Section',
                            $bootSectionSegment->startAddress,
                            $bootSectionSegment->size,
                            []
                        )
                    ],
                    $appSectionSegment->executable
                )
            ];
        }

        return $output;
    }

    private function memorySegmentFromElement(DOMElement $element): MemorySegment
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new MemorySegment(
            isset($attributes['name']) ? strtolower($attributes['name']) : null,
            $attributes['name'] ?? null,
            $this->resolveMemorySegmentType(
                $attributes['type'] ?? '',
                $attributes['name'] ?? ''
            ),
            $this->stringService->tryStringToInt($attributes['start'] ?? null),
            $this->stringService->tryStringToInt($attributes['size'] ?? null),
            $this->stringService->tryStringToInt($attributes['pagesize'] ?? null),
            $attributes['rw'] ?? null,
            [],
            isset($attributes['exec']) && $attributes['exec']
        );
    }

    private function resolveMemorySegmentType(string $atdfTypeName, string $atdfSegmentName): ?MemorySegmentType
    {
        $atdfTypeName = strtolower($atdfTypeName);
        $atdfSegmentName = strtolower($atdfSegmentName);

        if (
            $atdfTypeName === 'aliased'
            || ($atdfTypeName === 'other' && str_starts_with($atdfSegmentName, 'mapped_'))
        ) {
            return MemorySegmentType::ALIASED;
        }

        if (
            $atdfTypeName === 'production_signatures'
            || ($atdfTypeName === 'other' && $atdfSegmentName === 'prod_signatures')
        ) {
            return MemorySegmentType::PRODUCTION_SIGNATURES;
        }

        return match ($atdfTypeName) {
            'gp_registers', 'regs' => MemorySegmentType::GENERAL_PURPOSE_REGISTERS,
            'eeprom' => MemorySegmentType::EEPROM,
            'flash' => MemorySegmentType::FLASH,
            'fuses' => MemorySegmentType::FUSES,
            'io' => MemorySegmentType::IO,
            'ram' => MemorySegmentType::RAM,
            'lockbits' => MemorySegmentType::LOCKBITS,
            'osccal' => MemorySegmentType::OSCCAL,
            'signatures' => MemorySegmentType::SIGNATURES,
            'user_signatures' => MemorySegmentType::USER_SIGNATURES,
            default => null
        };
    }

    private function physicalInterfaceFromElement(DOMElement $element): PhysicalInterface
    {
        $physicalInterfacesByName = [
            'isp' => TargetPhysicalInterface::ISP,
            'debugwire' => TargetPhysicalInterface::DEBUG_WIRE,
            'updi' => TargetPhysicalInterface::UPDI,
            'pdi' => TargetPhysicalInterface::PDI,
            'jtag' => TargetPhysicalInterface::JTAG,
        ];

        $attributes = $this->getNodeAttributesByName($element);
        return new PhysicalInterface(
            $physicalInterfacesByName[strtolower($attributes['name'] ?? '')]?->value
                ?? (!empty($attributes['type']) ? strtolower($attributes['type']) : null),
            []
        );
    }

    private function peripheralFromElement(DOMElement $element, array $moduleAttributes): Peripheral
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new Peripheral(
            isset($attributes['name']) ? strtolower($attributes['name']) : null,
            $attributes['name'] ?? null,
            strtolower(
                isset($moduleAttributes['id']) && isset($moduleAttributes['name'])
                    ? $moduleAttributes['id'] . '_' . $moduleAttributes['name']
                    : $moduleAttributes['id'] ?? $moduleAttributes['name'] ?? ''
            ),
            [],
            []
        );

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'register-group') {
                $output->registerGroupInstances[] = $this->registerGroupInstanceFromElement($childNode);
            }
        }

        if (count($output->registerGroupInstances) === 1) {
            /*
             * ATDF peripherals sometimes override the register group keys and names, when there's only a single
             * register group in the peripheral.
             *
             * This is not necessary for Bloom's TDFs, so we tidy it up here by clearing the `key` and `name`
             * attributes where we can.
             */
            foreach ($output->registerGroupInstances as $instance) {
                if ($output->key == 'fuse' && $instance->key !== $instance->registerGroupKey) {
                    continue;
                }

                $instance->key = null;
                $instance->name = null;
            }
        }

        $signalElement = $element->getElementsByTagName('signals')->item(0);
        if ($signalElement instanceof DOMElement) {
            foreach ($signalElement->childNodes as $childNode) {
                if (!$childNode instanceof DOMElement) {
                    continue;
                }

                if ($childNode->nodeName === 'signal') {
                    $output->signals[] = $this->signalFromElement($childNode);
                }
            }
        }

        return $output;
    }

    private function registerGroupInstanceFromElement(DOMElement $element): RegisterGroupInstance
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new RegisterGroupInstance(
            isset($attributes['name']) ? strtolower($attributes['name']) : null,
            $attributes['name'] ?? null,
            isset($attributes['name-in-module'])
                ? strtolower($attributes['name-in-module'])
                : null,
            isset($attributes['address-space'])
                ? strtolower($attributes['address-space'])
                : null,
            $this->stringService->tryStringToInt($attributes['offset'] ?? null),
            $attributes['caption'] ?? null
        );
    }

    private function signalFromElement(DOMElement $element): Signal
    {
        $attributes = $this->getNodeAttributesByName($element);

        $padKey = isset($attributes['pad']) ? strtolower(trim($attributes['pad'])) : null;
        $function = $attributes['function'] ?? null;
        return new Signal(
            isset($attributes['group'])
                ? trim($attributes['group'])
                : (!empty($padKey) ? strtoupper($padKey) : null),
            $padKey,
            stristr((string) $function, 'default') !== false
                ? false
                : (stristr((string) $function, '_alt') !== false ? true : null),
            $this->stringService->tryStringToInt($attributes['index'] ?? null),
            $function,
            $attributes['field'] ?? null
        );
    }

    private function moduleFromElement(DOMElement $element): Module
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new Module(
            strtolower(
                isset($attributes['id']) && isset($attributes['name'])
                    ? $attributes['id'] . '_' . $attributes['name']
                    : $attributes['id'] ?? $attributes['name'] ?? ''
            ),
            $attributes['name'] ?? null,
            $attributes['caption'] ?? null,
            []
        );

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'register-group') {
                $output->registerGroups = array_merge(
                    $output->registerGroups,
                    $this->registerGroupFromElement($childNode)
                );
            }
        }

        return $output;
    }

    private function registerGroupFromElement(DOMElement $element): array
    {
        $attributes = $this->getNodeAttributesByName($element);

        $modeElements = $element->getElementsByTagName('mode');
        if ($modeElements->count() > 0) {
            $parentGroup = new RegisterGroup(
                isset($attributes['name']) ? strtolower($attributes['name']) : null,
                $attributes['name'] ?? null,
                $this->stringService->tryStringToInt($attributes['offset'] ?? null),
                [],
                [],
                []
            );

            /** @var RegisterGroup[] $modeGroupsByName */
            $modeGroupsByName = [];

            foreach ($modeElements as $modeElement) {
                $modeAttributes = $this->getNodeAttributesByName($modeElement);
                $modeGroup = new RegisterGroup(
                    isset($modeAttributes['name']) && $parentGroup->key !== null
                        ? $parentGroup->key . '_' . strtolower($modeAttributes['name'])
                        : null,
                    $modeAttributes['name'] ?? null,
                    null,
                    [],
                    [],
                    []
                );
                $modeGroupsByName[$modeGroup->name] = $modeGroup;

                $parentGroup->subgroupReferences[] = new RegisterGroupReference(
                    $modeGroup->key,
                    $modeGroup->name,
                    $modeGroup->key,
                    0,
                    null
                );
            }

            foreach ($element->childNodes as $childNode) {
                if (!$childNode instanceof DOMElement) {
                    continue;
                }

                if ($childNode->nodeName === 'register') {
                    $registerAttributes = $this->getNodeAttributesByName($childNode);
                    if (
                        isset($registerAttributes['modes'])
                        && array_key_exists($registerAttributes['modes'], $modeGroupsByName)
                    ) {
                        $modeGroupsByName[$registerAttributes['modes']]->registers[] = $this->registerFromElement(
                            $childNode
                        );

                    } else {
                        $parentGroup->registers[] = $this->registerFromElement($childNode);
                    }

                    continue;
                }

                if ($childNode->nodeName === 'register-group') {
                    $subgroupAttributes = $this->getNodeAttributesByName($childNode);
                    if (
                        isset($subgroupAttributes['modes'])
                        && array_key_exists($subgroupAttributes['modes'], $modeGroupsByName)
                    ) {
                        if (isset($subgroupAttributes['name-in-module'])) {
                            $modeGroupsByName[$subgroupAttributes['modes']]->subgroupReferences[] =
                                $this->registerGroupReferenceFromElement(
                                    $childNode
                                );

                        } else {
                            $modeGroupsByName[$subgroupAttributes['modes']]->subgroups = array_merge(
                                $modeGroupsByName[$subgroupAttributes['modes']]->subgroups,
                                $this->registerGroupFromElement($childNode)
                            );
                        }

                    } else {
                        if (isset($subgroupAttributes['name-in-module'])) {
                            $parentGroup->subgroupReferences[] = $this->registerGroupReferenceFromElement($childNode);

                        } else {
                            $parentGroup->subgroups = array_merge(
                                $parentGroup->subgroups,
                                $this->registerGroupFromElement($childNode)
                            );
                        }
                    }
                }
            }

            return array_merge(array_values($modeGroupsByName), [$parentGroup]);
        }

        $output = new RegisterGroup(
            isset($attributes['name']) ? strtolower($attributes['name']) : null,
            $attributes['name'] ?? null,
            $this->stringService->tryStringToInt($attributes['offset'] ?? null),
            [],
            [],
            []
        );

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'register') {
                $output->registers[] = $this->registerFromElement($childNode);
                continue;
            }

            if ($childNode->nodeName === 'register-group') {
                if (isset($this->getNodeAttributesByName($childNode)['name-in-module'])) {
                    $output->subgroupReferences[] = $this->registerGroupReferenceFromElement($childNode);

                } else {
                    $output->subgroups = array_merge($output->subgroups, $this->registerGroupFromElement($childNode));
                }
            }
        }

        return [$output];
    }

    private function registerGroupReferenceFromElement(DOMElement $element): RegisterGroupReference
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new RegisterGroupReference(
            isset($attributes['name']) ? strtolower($attributes['name']) : null,
            $attributes['name'] ?? null,
            isset($attributes['name-in-module']) ? strtolower($attributes['name-in-module']) : null,
            $this->stringService->tryStringToInt($attributes['offset'] ?? null),
            $attributes['caption'] ?? null
        );
    }

    private function registerFromElement(DOMElement $element): Register
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new Register(
            isset($attributes['name']) ? strtolower($attributes['name']) : null,
            $attributes['name'] ?? null,
            $attributes['caption'] ?? null,
            $this->stringService->tryStringToInt($attributes['offset'] ?? null),
            $this->stringService->tryStringToInt($attributes['size'] ?? null),
            $this->stringService->tryStringToInt($attributes['intval'] ?? null),
            $attributes['rw'] ?? null,
            null,
            []
        );

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'bitfield') {
                $output->bitFields[] = $this->bitFieldFromElement($childNode);
            }
        }

        return $output;
    }

    private function bitFieldFromElement(DOMElement $element): BitField
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new BitField(
            isset($attributes['name']) ? strtolower($attributes['name']) : null,
            $attributes['name'] ?? null,
            $attributes['caption'] ?? null,
            $this->stringService->tryStringToInt($attributes['mask'] ?? null),
            $attributes['rw'] ?? null
        );
    }

    private function padFromPinElement(DOMElement $element): ?Pad
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new Pad(
            isset($attributes['pad']) ? strtolower(trim($attributes['pad'])) : null,
            trim($attributes['pad'] ?? '')
        );
    }

    private function pinoutFromElement(DOMElement $element): Pinout
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new Pinout(
            isset($attributes['name']) ? strtolower($attributes['name']) : null,
            $attributes['name'] ?? null,
            null,
            $attributes['function'] ?? null,
            []
        );

        if (stristr($output->name, PinoutType::DIP->value) !== false) {
            $output->type = PinoutType::DIP;

        } elseif (stristr($output->name, PinoutType::SOIC->value) !== false) {
            $output->type = PinoutType::SOIC;

        } elseif (stristr($output->name, PinoutType::SSOP->value) !== false) {
            $output->type = PinoutType::SSOP;

        } elseif (stristr($output->name, PinoutType::DUAL_ROW_QFN->value) !== false) {
            $output->type = PinoutType::DUAL_ROW_QFN;

        }  elseif (stristr($output->name, PinoutType::QFN->value) !== false) {
            $output->type = PinoutType::QFN;

        }  elseif (stristr($output->name, PinoutType::MLF->value) !== false) {
            $output->type = PinoutType::MLF;

        } elseif (stristr($output->name, PinoutType::QFP->value) !== false) {
            $output->type = PinoutType::QFP;

        }  elseif (stristr($output->name, PinoutType::BGA->value) !== false) {
            $output->type = PinoutType::BGA;
        }

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'pin') {
                $output->pins[] = $this->pinFromElement($childNode);
            }
        }

        return $output;
    }

    private function pinFromElement(DOMElement $element): Pin
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new Pin(
            $attributes['position'] ?? null,
            ($pad = strtolower(trim($attributes['pad'] ?? ''))) !== 'nc' ? $pad : null
        );
    }

    private function variantFromElement(DOMElement $element): Variant
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new Variant(
            str_replace(
                '-',
                '_',
                strtolower($attributes['ordercode'] ?? $attributes['name'] ?? '')
            ),
            $attributes['ordercode'] ?? $attributes['name'] ?? null,
            isset($attributes['pinout']) ? strtolower($attributes['pinout']) : null,
            []
        );
    }
}
