<?php

use Targets\TargetDescriptionFiles\Services\Xml\AtdfService;
use Targets\TargetDescriptionFiles\Avr8\Services\ValidationService;
use Targets\TargetDescriptionFiles\Services\Xml\XmlService;

/*
 * This script makes use of the AtdfService class to convert Microchip ATDF files to Bloom's TDF format.
 *
 * Upon conversion, the script will also run validation on the newly generated TDFs, and output any failures.
 * Validation failures will *not* prevent the saving of the newly generated TDFs.
 *
 * Usage:
 *   To convert a single ATDF file, located at [PATH_TO_ATDF], and store the generated TDF in [OUTPUT_DIR_PATH]:
 *     php ./ConvertAtdfToTdf.php [PATH_TO_ATDF].atdf [OUTPUT_DIR_PATH]
 *
 *   To convert all ATDF files located in [PATH_TO_ATDF_DIR] (recursive search), and store the generated TDF files
 *   in [OUTPUT_DIR_PATH]:
 *     php ./ConvertAtdfToTdf.php [PATH_TO_ATDF_DIR] [OUTPUT_DIR_PATH]
 *
 * As mentioned in the AtdfService, this script is only intended for first-passes. It will do most of the work, but the
 * newly generated TDFs may require some manual touch-ups. This is why we run them through validation.
 */
define('ATDF_PATH', $argv[1] ?? null);
define('TDF_OUTPUT_PATH', $argv[2] ?? null);

if (empty(ATDF_PATH)) {
    print 'Missing ATDF path. Aborting\n';
    exit(1);
}

require_once __DIR__ . '/Targets/TargetDescriptionFiles/Services/AtdfService.php';
require_once __DIR__ . '/Targets/TargetDescriptionFiles/Services/Xml/XmlService.php';
require_once __DIR__ . '/Targets/TargetDescriptionFiles/Services/ValidationService.php';
require_once __DIR__ . '/Targets/TargetDescriptionFiles/Avr8/Services/ValidationService.php';

$atdfService = new AtdfService();
$avrValidationService = new ValidationService();
$xmlService = new XmlService();

$atdfFiles = [];

if (is_dir(ATDF_PATH)) {
    $atdfFiles = $atdfService->findFiles(ATDF_PATH);
    print count($atdfFiles) . ' ATDFs found in ' . ATDF_PATH . PHP_EOL . PHP_EOL;

} else {
    $atdfFiles = [new \SplFileInfo(ATDF_PATH)];
}

foreach ($atdfFiles as $atdfFile) {
    $atdfFilePath = $atdfFile->getPathname();

    print 'Processing ' . $atdfFilePath . PHP_EOL;

    $xmlDocument = new \DOMDocument();
    $xmlDocument->load($atdfFilePath);

    $targetDescriptionFile = $atdfService->toTdf($xmlDocument);
    $validationFailures = $avrValidationService->validateAvr8Tdf($targetDescriptionFile);

    if (empty($validationFailures)) {
        print "\033[32m";
        print 'Validation passed';

    } else {
        print "\033[31m";
        print 'Validation for ' . $atdfFilePath . ' failed' . PHP_EOL;
        print count($validationFailures) . ' error(s) found:' . PHP_EOL;
        print implode(PHP_EOL, $validationFailures);
    }

    print PHP_EOL . PHP_EOL;

    print "\033[0m";

    if (!file_exists(TDF_OUTPUT_PATH)) {
        mkdir(TDF_OUTPUT_PATH, 0700, true);
    }

    $document = $xmlService->toXml($targetDescriptionFile);
    $document->formatOutput = true;
    file_put_contents(
        TDF_OUTPUT_PATH . '/' . strtoupper($targetDescriptionFile->getConfigurationValue()) . '.xml',
        $document->saveXML()
    );
}

print 'Done' . PHP_EOL;
