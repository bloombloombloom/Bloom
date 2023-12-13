<?php

namespace Bloom\BuildScripts;

define('TDF_PATH', $argv[1] ?? null);

if (empty(TDF_PATH)) {
    print 'Missing TDF path. Aborting\n';
    exit(1);
}

require_once __DIR__ . '/TargetDescriptionFiles/Factory.php';

$xmlFiles = [];

if (is_dir(TDF_PATH)) {
    $xmlFiles = TargetDescriptionFiles\Factory::findXmlFiles(TDF_PATH);
    print count($xmlFiles) . ' target descriptions files found in ' . TDF_PATH . PHP_EOL . PHP_EOL;

} else {
    $xmlFiles = [new \SplFileInfo(TDF_PATH)];
}

$processedTargetConfigValues = [];
$failedValidationCount = 0;

foreach ($xmlFiles as $xmlFile) {
    $xmlFilePath = $xmlFile->getPathname();

    print 'Processing ' . $xmlFilePath . PHP_EOL;
    $targetDescriptionFile = TargetDescriptionFiles\Factory::loadTdfFromFile($xmlFilePath);

    $validationFailures = $targetDescriptionFile->validate();
    if (in_array($targetDescriptionFile->configurationValue, $processedTargetConfigValues)) {
        $validationFailures[] = 'Duplicate target configuration value ("'
            . $targetDescriptionFile->configurationValue . '")';
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

    $processedTargetConfigValues[] = $targetDescriptionFile->configurationValue;
}

print 'Validated ' . count($xmlFiles) . ' TDFs' . PHP_EOL;
print (($failedValidationCount > 0) ? "\033[31m" : "\033[32m");
print $failedValidationCount . ' failure(s)' . "\033[0m" . PHP_EOL;
print 'Done' . PHP_EOL;

if ($failedValidationCount > 0) {
    exit(1);
}
