<?php

use Targets\TargetDescriptionFiles\Services\DiscoveryService;
use Targets\TargetDescriptionFiles\Services\Xml\XmlService;
use Targets\TargetDescriptionFiles\Services\ValidationService;
use Targets\TargetDescriptionFiles\Avr8\Avr8TargetDescriptionFile;

define('TDF_PATH', $argv[1] ?? null);

if (empty(TDF_PATH)) {
    print 'Missing TDF path. Aborting\n';
    exit(1);
}

if (!file_exists(TDF_PATH)) {
    print 'Invalid TDF path - "' . TDF_PATH . '" does not exist' . PHP_EOL;
    exit(1);
}

require_once __DIR__ . '/Targets/TargetDescriptionFiles/Services/DiscoveryService.php';
require_once __DIR__ . '/Targets/TargetDescriptionFiles/Services/Xml/XmlService.php';
require_once __DIR__ . '/Targets/TargetDescriptionFiles/Services/ValidationService.php';
require_once __DIR__ . '/Targets/TargetDescriptionFiles/AVR8/Services/ValidationService.php';

$xmlService = new XmlService();

$validationService = new ValidationService();
$avrValidationService = new \Targets\TargetDescriptionFiles\AVR8\Services\ValidationService();

$xmlFiles = [];

if (is_dir(TDF_PATH)) {
    $discoveryService = new DiscoveryService();

    $xmlFiles = $discoveryService->findTargetDescriptionFiles(TDF_PATH);
    print count($xmlFiles) . ' target descriptions files found in ' . TDF_PATH . PHP_EOL . PHP_EOL;

} else {
    $xmlFiles = [new \SplFileInfo(TDF_PATH)];
}

$processedTargetConfigValues = [];
$failedValidationCount = 0;

foreach ($xmlFiles as $xmlFile) {
    $xmlFilePath = $xmlFile->getPathname();

    print 'Processing ' . $xmlFilePath . PHP_EOL;

    $xmlDocument = new \DOMDocument();
    $xmlDocument->load($xmlFilePath);
    $targetDescriptionFile = $xmlService->fromXml($xmlDocument);

    $validationFailures = $targetDescriptionFile instanceof Avr8TargetDescriptionFile
        ? $avrValidationService->validateAvr8Tdf($targetDescriptionFile)
        : $validationService->validateTdf($targetDescriptionFile);

    if (in_array($targetDescriptionFile->getConfigurationValue(), $processedTargetConfigValues)) {
        $validationFailures[] = 'Duplicate target configuration value ("'
            . $targetDescriptionFile->getConfigurationValue() . '")';
    }

    if (!empty($validationFailures)) {
        $failedValidationCount++;

        print "\033[31m";
        print 'Validation for ' . $xmlFilePath . ' failed' . PHP_EOL;
        print count($validationFailures) . ' error(s) found:' . PHP_EOL;
        print implode(PHP_EOL, $validationFailures);
        print PHP_EOL . PHP_EOL;

    } else {
        print "\033[32m";
        print 'Validation for ' . $xmlFilePath . ' passed' . PHP_EOL;
    }

    print "\033[0m";

    $processedTargetConfigValues[] = $targetDescriptionFile->getConfigurationValue();
}

print 'Validated ' . count($xmlFiles) . ' TDFs' . PHP_EOL;
print (($failedValidationCount > 0) ? "\033[31m" : "\033[32m");
print $failedValidationCount . ' failure(s)' . "\033[0m" . PHP_EOL;
print 'Done' . PHP_EOL;

if ($failedValidationCount > 0) {
    exit(1);
}
