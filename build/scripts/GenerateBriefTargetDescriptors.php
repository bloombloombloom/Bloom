<?php

use Targets\TargetDescriptionFiles\Services\DiscoveryService;
use Targets\TargetDescriptionFiles\Services\Xml\XmlService;
use Targets\TargetDescriptionFiles\TargetFamily;

define('TDF_DIR_PATH', $argv[1] ?? null);
define('OUTPUT_PATH', $argv[2] ?? null);
define('TDF_OUTPUT_PATH', $argv[3] ?? null);

if (empty(TDF_DIR_PATH)) {
    print 'Missing TDF directory path. Aborting' . PHP_EOL;
    exit(1);
}

if (!file_exists(TDF_DIR_PATH)) {
    print 'Invalid TDF directory path - "' . TDF_DIR_PATH . '" does not exist' . PHP_EOL;
    exit(1);
}

if (empty(OUTPUT_PATH)) {
    print 'Missing TDF mapping output path. Aborting' . PHP_EOL;
    exit(1);
}

if (empty(TDF_OUTPUT_PATH)) {
    print 'Missing TDF output path. Aborting' . PHP_EOL;
    exit(1);
}

if (!file_exists(dirname(OUTPUT_PATH))) {
    mkdir(dirname(OUTPUT_PATH), 0700, true);
}

require_once __DIR__ . '/Targets/TargetDescriptionFiles/Services/DiscoveryService.php';
require_once __DIR__ . '/Targets/TargetDescriptionFiles/Services/Xml/XmlService.php';
require_once __DIR__ . '/Targets/TargetDescriptionFiles/TargetFamily.php';

require_once __DIR__ . '/Targets/TargetDescriptionFiles/Avr8/Avr8TargetDescriptionFile.php';

$discoveryService = new DiscoveryService();
$xmlService = new XmlService();

$xmlFiles = $discoveryService->findTargetDescriptionFiles(TDF_DIR_PATH);
print count($xmlFiles) . ' target description files found in ' . TDF_DIR_PATH . PHP_EOL . PHP_EOL;

const MAP_ENTRY_TEMPLATE = '{
    "@CONFIG_VALUE@",
    {
        .name = "@TARGET_NAME@",
        .configValue = "@CONFIG_VALUE@",
        .family = @TARGET_FAMILY@,
        .relativeTdfPath = "@TDF_PATH@"
    }
}';

$entries = [];

foreach ($xmlFiles as $xmlFile) {
    $xmlFilePath = $xmlFile->getPathname();

    print 'Processing ' . $xmlFilePath . PHP_EOL;

    $xmlDocument = new \DOMDocument();
    $xmlDocument->load($xmlFilePath);
    $targetDescriptionFile = $xmlService->fromXml($xmlDocument);

    $relativeTdfPath = $targetDescriptionFile->getFamily()->value . '/'
        . strtoupper($targetDescriptionFile->getName()) . '.xml';

    $entries[] = str_replace(
        ['@CONFIG_VALUE@', '@TARGET_NAME@', '@TARGET_FAMILY@', '@TDF_PATH@'],
        [
            $targetDescriptionFile->getConfigurationValue(),
            $targetDescriptionFile->getName(),
            match ($targetDescriptionFile->getFamily()) {
                TargetFamily::AVR_8 => 'Targets::TargetFamily::AVR_8',
                TargetFamily::RISC_V => 'Targets::TargetFamily::RISC_V',
            },
            $relativeTdfPath,
        ],
        MAP_ENTRY_TEMPLATE
    );

    $tdfDestinationPath = TDF_OUTPUT_PATH . '/' . $relativeTdfPath;

    $tdfDestinationDirPath = dirname($tdfDestinationPath);
    if (!file_exists($tdfDestinationDirPath)) {
        mkdir($tdfDestinationDirPath, 0700, true);
    }

    if (!copy($xmlFilePath, $tdfDestinationPath)) {
        print 'FATAL ERROR: Failed to copy TDF file to ' . $tdfDestinationPath . PHP_EOL;
        print 'Aborting' . PHP_EOL;
        exit(1);
    }
}

file_put_contents(OUTPUT_PATH, implode(',' . PHP_EOL, $entries));

print PHP_EOL;
print 'Processed ' . count($xmlFiles) . ' TDFs.' . PHP_EOL;
print 'Generated brief target descriptors at ' . OUTPUT_PATH . PHP_EOL;
print 'Done' . PHP_EOL;
