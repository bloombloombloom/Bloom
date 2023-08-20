<?php
/*
 * Copies AVR8 target description files to AVR_TDF_DEST_FILE_PATH, in preparation for a build, and creates a JSON
 * mapping of target signatures to file paths (relative to Bloom's resource directory).
 * The JSON mapping is compiled as a Qt resource and used for looking-up target description file paths, by target ID.
 *
 * This script should be run as part of the build process.
 */

namespace Bloom\BuildScripts;

$buildPath = $argv[1] ?? null;
if (empty($buildPath)) {
    print "Missing build path. Aborting\n";
    die;
}

require_once __DIR__ . "/TargetDescriptionFiles/Factory.php";

define("AVR_TDF_DEST_FILE_PATH", $buildPath . "/resources/TargetDescriptionFiles/AVR");
define("AVR_TDF_DEST_RELATIVE_FILE_PATH", "TargetDescriptionFiles/AVR");
define("AVR_TDF_MAPPING_FILE_PATH", AVR_TDF_DEST_FILE_PATH . "/Mapping.json");

// Empty destination directory
if (file_exists(AVR_TDF_DEST_FILE_PATH)) {
    // There is no PHP function to delete a non-empty directory and I can't be arsed to write one. Bite me
    exec("rm -r " . AVR_TDF_DEST_FILE_PATH);
}

if (file_exists(AVR_TDF_MAPPING_FILE_PATH)) {
    unlink(AVR_TDF_MAPPING_FILE_PATH);
}

mkdir(AVR_TDF_DEST_FILE_PATH, 0700, true);

print "Loading AVR8 TDFs\n\n";

$tdfMapping = [];
$avrTdfs = TargetDescriptionFiles\Factory::loadAvr8Tdfs();

print "Processing " . count($avrTdfs) . " AVR8 TDFs...\n\n";

foreach ($avrTdfs as $avrTdf) {
    print "Processing AVR8 TDF for target " . $avrTdf->targetName . "\n";

    $strippedTargetName = str_replace([' '] , '_', $avrTdf->targetName);
    $id = strtolower($strippedTargetName);

    if (in_array($id, $tdfMapping)) {
        print "\033[31m" . "\n";
        print "FATAL ERROR: duplicate AVR8 target ID detected: " . $id . "\n\n"
            . "TDF Path: " . realpath($avrTdf->filePath);
        print "\033[0m" . "\n";
        exit(1);
    }

    if (!empty(($validationFailures = $avrTdf->validate()))) {
        print "\033[31m" . "\n";
        print "FATAL ERROR: AVR8 TDF failed validation - failure reasons:" . "\n"
            . implode("\n", $validationFailures) . "\n\n" . "TDF Path: "
            . realpath($avrTdf->filePath);
        print "\033[0m" . "\n";
        exit(1);
    }

    $destinationFilePath = AVR_TDF_DEST_FILE_PATH;
    $relativeDestinationFilePath = AVR_TDF_DEST_RELATIVE_FILE_PATH;

    if (!empty($avrTdf->targetArchitecture)) {
        // Group by architecture
        $destinationFilePath .= "/" . strtoupper($avrTdf->targetArchitecture);
        $relativeDestinationFilePath .= "/" . strtoupper($avrTdf->targetArchitecture);
    }

    $avrFamily = $avrTdf->getFamily();
    if (!empty($avrFamily)) {
        // Group by family
        $destinationFilePath .= "/" . str_replace([' '] , '_', strtoupper($avrFamily));
        $relativeDestinationFilePath .= "/" . str_replace([' '] , '_', strtoupper($avrFamily));
    }

    if (!file_exists($destinationFilePath)) {
        mkdir($destinationFilePath, 0700, true);
    }

    $destinationFilePath .= "/" . strtoupper($strippedTargetName) . ".xml";
    $relativeDestinationFilePath .= "/" . strtoupper($strippedTargetName) . ".xml";

    // Copy TDF to build location
    if (copy($avrTdf->filePath, $destinationFilePath) === false) {
        print "FATAL ERROR: Failed to copy TDF file to " . $destinationFilePath . "\n";
        print "Aborting\n";
        exit(1);
    }

    $tdfMapping[$id] = [
        'name' => $strippedTargetName,
        'signature' => $avrTdf->getSignature()->toHex(),
        'tdfPath' => $relativeDestinationFilePath,
    ];
}

if (file_put_contents(AVR_TDF_MAPPING_FILE_PATH, json_encode($tdfMapping, JSON_PRETTY_PRINT)) === false) {
    print "FATAL ERROR: Failed to create JSON mapping of target IDs to target description file paths\n";
    exit(1);
}

print "\n";
print "Created JSON mapping of target IDs to target description file paths: " . AVR_TDF_MAPPING_FILE_PATH . "\n\n";
print "Processed " . count($avrTdfs) . " files.\n";
print "Done\n";
