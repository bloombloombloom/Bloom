<?php
namespace Targets\TargetDescriptionFiles\Services\Xml;

use DOMDocument;
use DOMElement;
use Targets\TargetDescriptionFiles\AddressSpace;
use Targets\TargetDescriptionFiles\BitField;
use Targets\TargetDescriptionFiles\MemorySegment;
use Targets\TargetDescriptionFiles\MemorySegmentSection;
use Targets\TargetDescriptionFiles\Module;
use Targets\TargetDescriptionFiles\Peripheral;
use Targets\TargetDescriptionFiles\PhysicalInterface;
use Targets\TargetDescriptionFiles\Pin;
use Targets\TargetDescriptionFiles\Pinout;
use Targets\TargetDescriptionFiles\Property;
use Targets\TargetDescriptionFiles\PropertyGroup;
use Targets\TargetDescriptionFiles\Register;
use Targets\TargetDescriptionFiles\RegisterGroup;
use Targets\TargetDescriptionFiles\RegisterGroupReference;
use Targets\TargetDescriptionFiles\Services\StringService;
use Targets\TargetDescriptionFiles\Signal;
use Targets\TargetDescriptionFiles\Variant;

require_once __DIR__ . '/../StringService.php';
require_once __DIR__ . '/../../TargetDescriptionFile.php';
require_once __DIR__ . '/Exceptions/XmlParsingException.php';

/**
 * The ToXmlService facilitates the conversion of TargetDescriptionFile objects to XML documents.
 */
class ToXmlService
{
    private StringService $stringService;

    public function __construct(?StringService $stringService = null)
    {
        $this->stringService = $stringService ?? new StringService();
    }

    public function propertyGroupToXml(PropertyGroup $propertyGroup, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('property-group');
        $element->setAttribute('key', strtolower($propertyGroup->key));

        foreach ($propertyGroup->subPropertyGroups as $subPropertyGroup) {
            $element->append($this->propertyGroupToXml($subPropertyGroup, $document));
        }

        foreach ($propertyGroup->properties as $property) {
            $element->append($this->propertyToXml($property, $document));
        }

        return $element;
    }

    public function propertyToXml(Property $property, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('property');
        $element->setAttribute('key', strtolower($property->key));
        $element->setAttribute('value', $property->value);

        return $element;
    }

    public function addressSpaceToXml(AddressSpace $addressSpace, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('address-space');
        $element->setAttribute('key', strtolower($addressSpace->key));
        $element->setAttribute('start', $this->stringService->tryIntToHex($addressSpace->startAddress, 8));
        $element->setAttribute('size', $addressSpace->size);

        if (!empty($addressSpace->endianness)) {
            $element->setAttribute('endianness', strtolower($addressSpace->endianness));
        }

        foreach ($addressSpace->memorySegments as $memorySegment) {
            $element->append($this->memorySegmentToXml($memorySegment, $document));
        }

        return $element;
    }

    public function memorySegmentToXml(MemorySegment $memorySegment, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('memory-segment');
        $element->setAttribute('key', strtolower($memorySegment->key));
        $element->setAttribute('name', $memorySegment->name);
        $element->setAttribute('type', $memorySegment->type->value ?? '');
        $element->setAttribute('start', $this->stringService->tryIntToHex($memorySegment->startAddress, 8));
        $element->setAttribute('size', $memorySegment->size);

        if (!empty($memorySegment->pageSize)) {
            $element->setAttribute('page-size', $memorySegment->pageSize);
        }

        if (!empty($memorySegment->access)) {
            $element->setAttribute('rw', $memorySegment->access);
        }

        foreach ($memorySegment->sections as $segmentSection) {
            $element->append($this->memorySegmentSectionToXml($segmentSection, $document));
        }

        return $element;
    }

    public function memorySegmentSectionToXml(MemorySegmentSection $section, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('section');
        $element->setAttribute('key', strtolower($section->key));
        $element->setAttribute('name', $section->name);
        $element->setAttribute('start', $this->stringService->tryIntToHex($section->startAddress, 8));
        $element->setAttribute('size', $section->size);

        foreach ($section->subSections as $section) {
            $element->append($this->memorySegmentSectionToXml($section, $document));
        }

        return $element;
    }

    public function physicalInterfaceToXml(PhysicalInterface $physicalInterface, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('physical-interface');
        $element->setAttribute('name', $physicalInterface->name);
        $element->setAttribute('type', $physicalInterface->type);

        return $element;
    }

    public function peripheralToXml(Peripheral $peripheral, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('peripheral');
        $element->setAttribute('key', $peripheral->key);
        $element->setAttribute('name', $peripheral->name);
        $element->setAttribute('module-key', $peripheral->moduleKey);

        foreach ($peripheral->registerGroupReferences as $registerGroupReference) {
            $element->append($this->registerGroupReferenceToXml($registerGroupReference, $document));
        }

        if (!empty($peripheral->signals)) {
            $signalsElement = $document->createElement('signals');
            foreach ($peripheral->signals as $signal) {
                $signalsElement->append($this->signalToXml($signal, $document));
            }

            $element->append($signalsElement);
        }

        return $element;
    }

    public function signalToXml(Signal $signal, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('signal');
        $element->setAttribute('pad-id', $signal->padId);

        if ($signal->index !== null) {
            $element->setAttribute('index', $signal->index);
        }

        if ($signal->function !== null) {
            $element->setAttribute('function', $signal->function);
        }

        if ($signal->group !== null) {
            $element->setAttribute('group', $signal->group);
        }

        if ($signal->field !== null) {
            $element->setAttribute('field', $signal->field);
        }

        return $element;
    }

    public function moduleToXml(Module $module, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('module');
        $element->setAttribute('key', $module->key);
        $element->setAttribute('name', $module->name);
        $element->setAttribute('description', trim($module->description));

        foreach ($module->registerGroups as $registerGroup) {
            $element->append($this->registerGroupToXml($registerGroup, $document));
        }

        return $element;
    }

    public function registerGroupToXml(RegisterGroup $registerGroup, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('register-group');
        $element->setAttribute('key', $registerGroup->key);
        $element->setAttribute('name', $registerGroup->name);

        if ($registerGroup->offset !== null) {
            $element->setAttribute('offset', $this->stringService->tryIntToHex($registerGroup->offset));
        }

        /*
         * A register group can have registers, register groups (subgroups) and register group references as children.
         *
         * We want these to appear in the order of their offset, in the XML. So we group them into a single array,
         * sort them, then generate the XML elements.
         */

        $children = array_merge(
            $registerGroup->registers,
            $registerGroup->subGroups,
            $registerGroup->subGroupReferences
        );

        usort(
            $children,
            fn (
                RegisterGroup|RegisterGroupReference|Register $childA,
                RegisterGroup|RegisterGroupReference|Register $childB
            ): bool => $childA->offset > $childB->offset
        );

        foreach ($children as $child) {
            /**
             * @var RegisterGroup|RegisterGroupReference|Register $child
             */

            if ($child instanceof Register) {
                $element->append($this->registerToXml($child, $document));
                continue;
            }

            if ($child instanceof RegisterGroup) {
                $element->append($this->registerGroupToXml($child, $document));
                continue;
            }

            if ($child instanceof RegisterGroupReference) {
                $element->append($this->registerGroupReferenceToXml($child, $document));
            }
        }

        return $element;
    }

    public function registerGroupReferenceToXml(
        RegisterGroupReference $registerGroupReference,
        DOMDocument $document
    ): DOMElement {
        $element = $document->createElement('register-group-reference');
        $element->setAttribute('key', $registerGroupReference->key);
        $element->setAttribute('name', $registerGroupReference->name);

        if (!empty($registerGroupReference->description)) {
            $element->setAttribute('description', $registerGroupReference->description);
        }

        $element->setAttribute('register-group-key', $registerGroupReference->registerGroupKey);
        $element->setAttribute('offset', $this->stringService->tryIntToHex($registerGroupReference->offset));

        return $element;
    }

    public function registerToXml(Register $register, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('register');
        $element->setAttribute('key', $register->key);
        $element->setAttribute('name', $register->name);

        if (!empty($register->description)) {
            $element->setAttribute('description', trim($register->description));
        }

        $element->setAttribute('offset', $this->stringService->tryIntToHex($register->offset));
        $element->setAttribute('size', $register->size);

        if ($register->initialValue !== null) {
            $element->setAttribute('initial-value', $this->stringService->tryIntToHex($register->initialValue));
        }

        if (!empty($register->access)) {
            $element->setAttribute('rw', $register->access);
        }

        if ($register->alternative !== null) {
            $element->setAttribute('alternative', $register->alternative ? 'true' : 'false');
        }

        foreach ($register->bitFields as $bitField) {
            $element->append($this->bitFieldToXml($bitField, $document));
        }

        return $element;
    }

    public function bitFieldToXml(BitField $bitField, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('bit-field');
        $element->setAttribute('key', $bitField->key);
        $element->setAttribute('name', $bitField->name);

        if (!empty($bitField->description)) {
            $element->setAttribute('description', trim($bitField->description));
        }

        $element->setAttribute('mask', $this->stringService->tryIntToHex($bitField->mask));

        if (!empty($bitField->access)) {
            $element->setAttribute('rw', $bitField->access);
        }

        return $element;
    }

    public function pinoutToXml(Pinout $pinout, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('pinout');
        $element->setAttribute('key', $pinout->key);
        $element->setAttribute('name', $pinout->name);
        $element->setAttribute('type', $pinout->type->value ?? '');

        if (!empty($pinout->function)) {
            $element->setAttribute('function', $pinout->function);
        }

        foreach ($pinout->pins as $pin) {
            $element->append($this->pinToXml($pin, $document));
        }

        return $element;
    }

    public function pinToXml(Pin $pin, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('pin');
        $element->setAttribute('position', $pin->position);
        $element->setAttribute('pad', $pin->pad);

        return $element;
    }

    public function variantToXml(Variant $variant, DOMDocument $document): DOMElement
    {
        $element = $document->createElement('variant');
        $element->setAttribute('name', $variant->name);
        $element->setAttribute('pinout-key', $variant->pinoutKey);
        $element->setAttribute('package', $variant->package);

        return $element;
    }
}
