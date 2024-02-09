<?php
namespace Targets\TargetDescriptionFiles\Services\Xml;

use DOMDocument;
use DOMException;
use Targets\TargetDescriptionFiles\TargetDescriptionFile;
use Targets\TargetDescriptionFiles\Avr8\Avr8TargetDescriptionFile;
use Targets\TargetDescriptionFiles\Services\Xml\Exceptions\XmlParsingException;
use Targets\TargetDescriptionFiles\TargetFamily;

require_once __DIR__ . '/FromXmlService.php';
require_once __DIR__ . '/ToXmlService.php';
require_once __DIR__ . '/../../TargetDescriptionFile.php';

require_once __DIR__ . '/Exceptions/XmlParsingException.php';

/**
 * The XmlService provides conversion of TargetDescriptionFile objects to/from XML documents.
 *
 * This can be used to:
 *  - Construct a new instance of TargetDescriptionFile, from an XML document.
 *  - Convert a TargetDescriptionFile instance to an XML document.
 */
class XmlService
{
    private FromXmlService $fromXmlService;
    private ToXmlService $toXmlService;

    public function __construct(?FromXmlService $fromXmlService = null, ?ToXmlService $toXmlService = null)
    {
        $this->fromXmlService = $fromXmlService ?? new FromXmlService();
        $this->toXmlService = $toXmlService ?? new ToXmlService();
    }

    /**
     * Constructs a TargetDescriptionFile from the given XML document.
     *
     * @param DOMDocument $document
     *  The XML document from which to construct the TargetDescriptionFile.
     *
     * @return TargetDescriptionFile
     *  The constructed TargetDescriptionFile.
     */
    public function fromXml(DOMDocument $document): TargetDescriptionFile
    {
        $deviceElement = $this->fromXmlService->getDeviceElement($document);
        $deviceAttributesByName = $this->fromXmlService->getNodeAttributesByName($deviceElement);

        $targetFamily = TargetFamily::tryFrom($deviceAttributesByName['family'] ?? null);
        if (!$targetFamily instanceof TargetFamily) {
            throw new XmlParsingException('Failed to resolve target family - missing/invalid family attribute');
        }

        $tdf = match ($targetFamily) {
            TargetFamily::AVR_8 => new Avr8TargetDescriptionFile(),
            default => new TargetDescriptionFile(),
        };
        $tdf->deviceAttributesByName = $deviceAttributesByName;

        $propertyGroupElements = $this->fromXmlService->getDeviceElementsFromXPath(
            'property-groups/property-group',
            $document
        );
        foreach ($propertyGroupElements as $element) {
            $tdf->propertyGroups[] = $this->fromXmlService->propertyGroupFromElement($element);
        }

        $addressSpaceElements = $this->fromXmlService->getDeviceElementsFromXPath(
            'address-spaces/address-space',
            $document
        );
        foreach ($addressSpaceElements as $element) {
            $tdf->addressSpaces[] = $this->fromXmlService->addressSpaceFromElement($element);
        }

        $physicalInterfaceElements = $this->fromXmlService->getDeviceElementsFromXPath(
            'physical-interfaces/physical-interface',
            $document
        );
        foreach ($physicalInterfaceElements as $element) {
            $tdf->physicalInterfaces[] = $this->fromXmlService->physicalInterfaceFromElement($element);
        }

        $peripheralElements = $this->fromXmlService->getDeviceElementsFromXPath(
            'peripherals/peripheral',
            $document
        );
        foreach ($peripheralElements as $element) {
            $tdf->peripherals[] = $this->fromXmlService->peripheralFromElement($element);
        }

        $moduleElements = $this->fromXmlService->getDeviceElementsFromXPath(
            'modules/module',
            $document
        );
        foreach ($moduleElements as $element) {
            $tdf->modules[] = $this->fromXmlService->moduleFromElement($element);
        }

        $pinoutElements = $this->fromXmlService->getDeviceElementsFromXPath(
            'pinouts/pinout',
            $document
        );
        foreach ($pinoutElements as $element) {
            $tdf->pinouts[] = $this->fromXmlService->pinoutFromElement($element);
        }

        $variantElements = $this->fromXmlService->getDeviceElementsFromXPath(
            'variants/variant',
            $document
        );
        foreach ($variantElements as $element) {
            $tdf->variants[] = $this->fromXmlService->variantFromElement($element);
        }

        return $tdf;
    }

    /**
     * Generates an XML document from the given TDF.
     *
     * @param TargetDescriptionFile $tdf
     *  The TDF from which to generate the XML document.
     *
     * @return DOMDocument
     *  The generated XML document.
     *
     * @throws DOMException
     */
    public function toXml(TargetDescriptionFile $tdf): DOMDocument
    {
        $document = new DOMDocument('1.0', 'UTF-8');
        $deviceElement = $document->createElement('device');

        $deviceElement->setAttribute('name', $tdf->getName());
        $deviceElement->setAttribute('family', $tdf->getFamily()->value ?? '');
        $deviceElement->setAttribute('configuration-value', $tdf->getConfigurationValue());

        $architecture = $tdf->getArchitecture();
        if (!empty($architecture)) {
            $deviceElement->setAttribute('architecture', $architecture);
        }

        $vendor = $tdf->getVendor();
        if (!empty($vendor)) {
            $deviceElement->setAttribute('vendor', $vendor);
        }

        foreach ($tdf->getAdditionalDeviceAttributes() as $attrName => $attrValue) {
            $deviceElement->setAttribute($attrName, $attrValue);
        }

        $propertyGroupsElement = $document->createElement('property-groups');
        foreach ($tdf->propertyGroups as $propertyGroup) {
            $propertyGroupsElement->append($this->toXmlService->propertyGroupToXml($propertyGroup, $document));
        }

        $deviceElement->append($propertyGroupsElement);

        $addressSpacesElement = $document->createElement('address-spaces');
        foreach ($tdf->addressSpaces as $addressSpace) {
            $addressSpacesElement->append($this->toXmlService->addressSpaceToXml($addressSpace, $document));
        }

        $deviceElement->append($addressSpacesElement);

        $interfacesElement = $document->createElement('physical-interfaces');
        foreach ($tdf->physicalInterfaces as $interface) {
            $interfacesElement->append($this->toXmlService->physicalInterfaceToXml($interface, $document));
        }

        $deviceElement->append($interfacesElement);

        $peripheralsElement = $document->createElement('peripherals');
        foreach ($tdf->peripherals as $peripheral) {
            $peripheralsElement->append($this->toXmlService->peripheralToXml($peripheral, $document));
        }

        $deviceElement->append($peripheralsElement);

        $modulesElement = $document->createElement('modules');
        foreach ($tdf->modules as $module) {
            $modulesElement->append($this->toXmlService->moduleToXml($module, $document));
        }

        $deviceElement->append($modulesElement);

        $pinoutsElement = $document->createElement('pinouts');
        foreach ($tdf->pinouts as $pinout) {
            $pinoutsElement->append($this->toXmlService->pinoutToXml($pinout, $document));
        }

        $deviceElement->append($pinoutsElement);

        $variantsElement = $document->createElement('variants');
        foreach ($tdf->variants as $variant) {
            $variantsElement->append($this->toXmlService->variantToXml($variant, $document));
        }

        $deviceElement->append($variantsElement);

        $document->append($deviceElement);
        return $document;
    }
}
