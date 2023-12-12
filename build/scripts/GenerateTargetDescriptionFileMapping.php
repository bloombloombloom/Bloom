<?php

namespace Bloom\BuildScripts;

use Bloom\BuildScripts\TargetDescriptionFiles\TargetDescriptionFile;

define('TDF_DIR_PATH', $argv[1] ?? null);
define('MAPPING_TEMPLATE_PATH', $argv[2] ?? null);
define('MAPPING_OUTPUT_PATH', $argv[3] ?? null);
define('TDF_OUTPUT_PATH', $argv[4] ?? null);

if (empty(TDF_DIR_PATH)) {
    print 'Missing TDF directory path. Aborting' . PHP_EOL;
    exit(1);
}

if (empty(MAPPING_TEMPLATE_PATH)) {
    print 'Missing TDF mapping template path. Aborting' . PHP_EOL;
    exit(1);
}

if (empty(MAPPING_OUTPUT_PATH)) {
    print 'Missing TDF mapping output path. Aborting' . PHP_EOL;
    exit(1);
}

if (empty(TDF_OUTPUT_PATH)) {
    print 'Missing TDF output path. Aborting' . PHP_EOL;
    exit(1);
}

if (!file_exists(dirname(MAPPING_OUTPUT_PATH))) {
    mkdir(dirname(MAPPING_OUTPUT_PATH), 0700);
}

require_once __DIR__ . '/TargetDescriptionFiles/Factory.php';
require_once __DIR__ . '/TargetDescriptionFiles/AVR8/Avr8TargetDescriptionFile.php';

$xmlFiles = TargetDescriptionFiles\Factory::findXmlFiles(TDF_DIR_PATH);
print count($xmlFiles) . ' target descriptions files found in ' . TDF_DIR_PATH . PHP_EOL . PHP_EOL;

$targetFamiliesByArch = [
    TargetDescriptionFile::ARCHITECTURE_AVR8 => 'TargetFamily::AVR8',
];

const MAP_ENTRY_TEMPLATE = '{"@CONFIG_VALUE@", {"@TARGET_NAME@", "@CONFIG_VALUE@", @TARGET_FAMILY@, "@TDF_PATH@"}}';

$entries = [];

foreach ($xmlFiles as $xmlFile) {
    $xmlFilePath = $xmlFile->getPathname();

    print 'Processing ' . $xmlFilePath . PHP_EOL;
    $targetDescriptionFile = TargetDescriptionFiles\Factory::loadTdfFromFile($xmlFilePath);

    $relativeTdfPath = $targetDescriptionFile->targetArchitecture . '/'
        . strtoupper($targetDescriptionFile->targetName) . '.xml';

    $entries[] = str_replace(
        ['@CONFIG_VALUE@', '@TARGET_NAME@', '@TARGET_FAMILY@', '@TDF_PATH@'],
        [
            $targetDescriptionFile->configurationValue,
            $targetDescriptionFile->targetName,
            $targetFamiliesByArch[$targetDescriptionFile->targetArchitecture],
            $relativeTdfPath,
        ],
        MAP_ENTRY_TEMPLATE
    );

    $tdfDestinationPath = TDF_OUTPUT_PATH . '/' . $relativeTdfPath;

    $tdfDestinationDirPath = dirname($tdfDestinationPath);
    if (!file_exists($tdfDestinationDirPath)) {
        mkdir($tdfDestinationDirPath, 0700, true);
    }

    if (!copy($targetDescriptionFile->filePath, $tdfDestinationPath)) {
        print 'FATAL ERROR: Failed to copy TDF file to ' . $tdfDestinationPath . PHP_EOL;
        print 'Aborting' . PHP_EOL;
        exit(1);
    }
}

file_put_contents(
    MAPPING_OUTPUT_PATH,
    str_replace(
        '//@MAPPING_PLACEHOLDER@',
        implode(',' . PHP_EOL . str_repeat(' ', 12), $entries),
        file_get_contents(MAPPING_TEMPLATE_PATH)
    )
);

print PHP_EOL;
print 'Processed ' . count($xmlFiles) . ' TDFs.' . PHP_EOL;
print 'Generated TDF mapping at ' . MAPPING_OUTPUT_PATH . PHP_EOL;
print 'Done' . PHP_EOL;
