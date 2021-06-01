<?php

require_once __DIR__ . "/TargetDescriptionFile.php";

const TDF_DIR_PATH = __DIR__ . "/../../resources/TargetDescriptionFiles";

/**
 * @param string $dirPath
 * @return SplFileInfo[]
 */
function getXmlFiles(string $dirPath): array
{
    $output = [];

    $directory = new DirectoryIterator($dirPath);
    foreach ($directory as $entry) {
        if ($entry->isFile() && $entry->getExtension() == 'xml') {
            $output[] = clone $entry;

        } else if ($entry->isDir() && !$entry->isDot()) {
            $output = array_merge($output, getXmlFiles($entry->getPathname()));
        }
    }

    return $output;
}

$xmlFiles = getXmlFiles(TDF_DIR_PATH);
$failedValidationCount = 0;

foreach ($xmlFiles as $xmlFile) {
    $targetDescriptionFile = new TargetDescriptionFile($xmlFile->getPathname());
    $validationFailures = $targetDescriptionFile->validate();

    if (!empty($validationFailures)) {
        $failedValidationCount++;

        print "\033[31m";
        print "Validation for " . $xmlFile->getFilename() . " failed.\n";
        print "Full path: " . $xmlFile->getRealPath() . "\n";
        print count($validationFailures) . " errors found:\n";
        print implode("\n", $validationFailures);
        print "\n\n";
        print "\033[0m";

    } else {
        print "\033[32m";
        print "Validation for " . $xmlFile->getFilename() . " passed.\n";
        print "\033[0m";
    }
}

print "\n\n";
print "Validated " . count($xmlFiles) . " TDFs. ";
print (($failedValidationCount > 0) ? "\033[31m" : "\033[32m");
print $failedValidationCount . " failures." . "\033[0m" . "\n";
echo "Done\n";
