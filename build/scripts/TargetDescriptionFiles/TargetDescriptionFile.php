<?php

require_once __DIR__ . "/Variant.php";

class TargetDescriptionFile
{
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

    private function init()
    {
        if (!file_exists($this->filePath)) {
            throw new Exception("Invalid TDF file path - file does not exist.");
        }

        $xml = simplexml_load_file($this->filePath);
        if ($xml === false) {
            throw new Exception("Failed to parse TDF XML.");
        }

        $this->xml = $xml;

        /** @var SimpleXMLElement[] $devices */
        $devices = (array) $xml->devices;
        if (!empty($devices)) {
            $device = reset($devices);
            $deviceAttributes = $device->attributes();

            if (!empty($deviceAttributes['name'])) {
                $this->targetName = $device['name'];
            }

            if (!empty($deviceAttributes['architecture'])) {
                $this->targetArchitecture = $device['architecture'];
            }
        }

        $variantElements = $xml->xpath('variants/variant');

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
