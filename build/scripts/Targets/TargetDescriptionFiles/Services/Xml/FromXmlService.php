<?php
namespace Targets\TargetDescriptionFiles\Services\Xml;

use DOMDocument;
use DOMNode;
use DOMNodeList;
use DOMElement;
use Targets\TargetDescriptionFiles\Services\StringService;
use Targets\TargetDescriptionFiles\Services\Xml\Exceptions\XmlParsingException;
use Targets\TargetDescriptionFiles\AddressSpace;
use Targets\TargetDescriptionFiles\BitField;
use Targets\TargetDescriptionFiles\MemorySegment;
use Targets\TargetDescriptionFiles\MemorySegmentSection;
use Targets\TargetDescriptionFiles\MemorySegmentType;
use Targets\TargetDescriptionFiles\Module;
use Targets\TargetDescriptionFiles\Peripheral;
use Targets\TargetDescriptionFiles\RegisterGroupInstance;
use Targets\TargetDescriptionFiles\PhysicalInterface;
use Targets\TargetDescriptionFiles\Property;
use Targets\TargetDescriptionFiles\PropertyGroup;
use Targets\TargetDescriptionFiles\Register;
use Targets\TargetDescriptionFiles\RegisterGroup;
use Targets\TargetDescriptionFiles\RegisterGroupReference;
use Targets\TargetDescriptionFiles\Signal;
use Targets\TargetDescriptionFiles\Pinout;
use Targets\TargetDescriptionFiles\PinoutType;
use Targets\TargetDescriptionFiles\Pin;
use Targets\TargetDescriptionFiles\Variant;

require_once __DIR__ . '/../StringService.php';
require_once __DIR__ . '/../../TargetDescriptionFile.php';
require_once __DIR__ . '/Exceptions/XmlParsingException.php';

/**
 * The FromXmlService facilitates the conversion of XML documents to TargetDescriptionFile objects.
 */
class FromXmlService
{
    private StringService $stringService;

    public function __construct(?StringService $stringService = null)
    {
        $this->stringService = $stringService ?? new StringService();
    }

    public function getDeviceElement(DOMDocument $document): DOMElement
    {
        $deviceElement = $document->getElementsByTagName('device')->item(0);
        if (!$deviceElement instanceof DOMElement) {
            throw new XmlParsingException('Missing device element');
        }

        return $deviceElement;
    }

    public function getNodeAttributesByName(DOMNode $node): array
    {
        $output = [];
        foreach ($node->attributes as $attribute) {
            /** @var \DOMAttr $attribute */
            $output[$attribute->name] = $attribute->value;
        }

        return $output;
    }

    public function getDeviceElementsFromXPath(string $expression, DOMDocument $document): DOMNodeList
    {
        $elements = (new \DOMXPath($document))->query('/device/' . $expression);

        if ($elements === false) {
            throw new \Exception('Failed to evaluate XPath expression - malformed expression');
        }

        return $elements;
    }

    public function propertyGroupFromElement(DOMElement $element): PropertyGroup
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new PropertyGroup($attributes['key'] ?? null, [], []);

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'property') {
                $output->properties[] = $this->propertyFromElement($childNode);
                continue;
            }

            if ($childNode->nodeName === 'property-group') {
                $output->subPropertyGroups[] = $this->propertyGroupFromElement($childNode);
            }
        }

        return $output;
    }

    public function propertyFromElement(DOMElement $element): Property
    {
        $attributes = $this->getNodeAttributesByName($element);
        return new Property(
            $attributes['key'] ?? null,
            $attributes['value'] ?? null,
        );
    }

    public function addressSpaceFromElement(DOMElement $element): AddressSpace
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new AddressSpace(
            $attributes['key'] ?? null,
            $this->stringService->tryStringToInt($attributes['start'] ?? null),
            $this->stringService->tryStringToInt($attributes['size'] ?? null),
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

        return $output;
    }

    public function memorySegmentFromElement(DOMElement $element): MemorySegment
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new MemorySegment(
            $attributes['key'] ?? null,
            $attributes['name'] ?? null,
            MemorySegmentType::tryFrom($attributes['type'] ?? null),
            $this->stringService->tryStringToInt($attributes['start'] ?? null),
            $this->stringService->tryStringToInt($attributes['size'] ?? null),
            $this->stringService->tryStringToInt($attributes['page-size'] ?? null),
            $attributes['access'] ?? null,
            [],
            isset($attributes['executable']) ? (bool) $attributes['executable'] : null
        );

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'section') {
                $output->sections[] = $this->memorySegmentSectionFromElement($childNode);
            }
        }

        return $output;
    }

    public function memorySegmentSectionFromElement(DOMElement $element): MemorySegmentSection
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new MemorySegmentSection(
            $attributes['key'] ?? null,
            $attributes['name'] ?? null,
            $this->stringService->tryStringToInt($attributes['start'] ?? null),
            $this->stringService->tryStringToInt($attributes['size'] ?? null),
            []
        );

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'section') {
                $output->subSections[] = $this->memorySegmentSectionFromElement($childNode);
            }
        }

        return $output;
    }

    public function physicalInterfaceFromElement(DOMElement $element): PhysicalInterface
    {
        $attributes = $this->getNodeAttributesByName($element);
        return new PhysicalInterface(
            $attributes['name'] ?? null,
            $attributes['type'] ?? null,
        );
    }

    public function moduleFromElement(DOMElement $element): Module
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new Module(
            $attributes['key'] ?? null,
            $attributes['name'] ?? null,
            $attributes['description'] ?? null,
            []
        );

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'register-group') {
                $output->registerGroups[] = $this->registerGroupFromElement($childNode);
            }
        }

        return $output;
    }

    public function registerGroupFromElement(DOMElement $element): RegisterGroup
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new RegisterGroup(
            $attributes['key'] ?? null,
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
                $output->subgroups[] = $this->registerGroupFromElement($childNode);
                continue;
            }

            if ($childNode->nodeName === 'register-group-reference') {
                $output->subgroupReferences[] = $this->registerGroupReferenceFromElement($childNode);
            }
        }

        return $output;
    }

    public function registerGroupReferenceFromElement(DOMElement $element): RegisterGroupReference
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new RegisterGroupReference(
            $attributes['key'] ?? null,
            $attributes['name'] ?? null,
            $attributes['register-group-key'] ?? null,
            $this->stringService->tryStringToInt($attributes['offset'] ?? null),
            $attributes['description'] ?? null
        );
    }

    public function registerFromElement(DOMElement $element): Register
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new Register(
            $attributes['key'] ?? null,
            $attributes['name'] ?? null,
            $attributes['description'] ?? null,
            $this->stringService->tryStringToInt($attributes['offset'] ?? null),
            $this->stringService->tryStringToInt($attributes['size'] ?? null),
            $this->stringService->tryStringToInt($attributes['initial-value'] ?? null),
            $attributes['access'] ?? null,
            isset($attributes['alternative']) ? trim($attributes['alternative']) === 'true' : null,
            []
        );

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'bit-field') {
                $output->bitFields[] = $this->bitFieldFromElement($childNode);
            }
        }

        return $output;
    }

    public function bitFieldFromElement(DOMElement $element): BitField
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new BitField(
            $attributes['key'] ?? null,
            $attributes['name'] ?? null,
            $attributes['description'] ?? null,
            $this->stringService->tryStringToInt($attributes['mask'] ?? null),
            $attributes['access'] ?? null
        );
    }

    public function peripheralFromElement(DOMElement $element): Peripheral
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new Peripheral(
            $attributes['key'] ?? null,
            $attributes['name'] ?? null,
            $attributes['module-key'] ?? null,
            [],
            []
        );

        foreach ($element->childNodes as $childNode) {
            if (!$childNode instanceof DOMElement) {
                continue;
            }

            if ($childNode->nodeName === 'register-group-instance') {
                $output->registerGroupInstances[] = $this->registerGroupInstanceFromElement($childNode);
            }
        }

        $signalsElements = $element->getElementsByTagName('signals');
        if ($signalsElements->count() > 1) {
            throw new XmlParsingException('Unexpected number of "signals" elements');
        }

        $signalElement = $signalsElements->item(0);
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

    public function registerGroupInstanceFromElement(DOMElement $element): RegisterGroupInstance
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new RegisterGroupInstance(
            $attributes['key'] ?? null,
            $attributes['name'] ?? null,
            $attributes['register-group-key'] ?? null,
            $attributes['address-space-key'] ?? null,
            $this->stringService->tryStringToInt($attributes['offset'] ?? null),
            $attributes['description'] ?? null
        );
    }

    public function signalFromElement(DOMElement $element): Signal
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new Signal(
            $attributes['pad-id'] ?? null,
            $this->stringService->tryStringToInt($attributes['index'] ?? null),
            $attributes['function'] ?? null,
            $attributes['group'] ?? null,
            $attributes['field'] ?? null
        );
    }

    public function pinoutFromElement(DOMElement $element): Pinout
    {
        $attributes = $this->getNodeAttributesByName($element);

        $output = new Pinout(
            $attributes['key'] ?? null,
            $attributes['name'] ?? null,
            PinoutType::tryFrom($attributes['type'] ?? null),
            $attributes['function'] ?? null,
            []
        );

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

    public function pinFromElement(DOMElement $element): Pin
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new Pin(
            $attributes['position'] ?? null,
            $attributes['pad'] ?? null
        );
    }

    public function variantFromElement(DOMElement $element): Variant
    {
        $attributes = $this->getNodeAttributesByName($element);

        return new Variant(
            $attributes['name'] ?? null,
            $attributes['pinout-key'] ?? null,
            $attributes['package'] ?? null
        );
    }
}
