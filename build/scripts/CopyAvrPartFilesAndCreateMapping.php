<?php
/*
 * Parses Microchip AVR Part Description (.atdf/.xml) files and creates a JSON mapping of target IDs to file paths.
 * Also copies the files over to the Distribution directory.
 *
 * The JSON mapping is compiled as a Qt resource and used for looking-up target description file paths, by target ID.
 *
 * This script should be run as part of the build process.
 */

CONST AVR_PD_FILE_PATH = __DIR__ . "/../../src/Targets/Microchip/AVR/PartDescriptionFiles";
CONST TARGET_PD_DEST_FILE_PATH = __DIR__ . "/../resources/TargetPartDescriptions/AVR";
CONST TARGET_PD_DEST_RELATIVE_FILE_PATH = "../resources/TargetPartDescriptions/AVR";
CONST TARGET_PD_MAPPING_FILE_PATH = TARGET_PD_DEST_FILE_PATH . "/Mapping.json";

CONST PD_COMMENT = "\n<!-- This is an automatically generated file. Any changes made to it will likely be lost. -->\n";

// Empty destination directory
if (file_exists(TARGET_PD_DEST_FILE_PATH)) {
    // There is no PHP function to delete a non-empty directory and I can't be arsed to write one. Bite me
    exec("rm -r " . TARGET_PD_DEST_FILE_PATH);
}

if (file_exists(TARGET_PD_MAPPING_FILE_PATH)) {
    unlink(TARGET_PD_MAPPING_FILE_PATH);
}

mkdir(TARGET_PD_DEST_FILE_PATH, 0777, true);

class TargetDescription implements JsonSerializable
{
    public ?string $targetId;
    public ?string $targetName;
    public ?string $originalFilePath;
    public ?string $destinationFilePath;
    public ?string $relativeDestinationFilePath;

    public function jsonSerialize() {
        return [
            'targetName' => $this->targetName,
            'targetDescriptionFilePath' => $this->relativeDestinationFilePath,
        ];
    }
}

$processedFileCount = 0;
$failedFileCount = 0;

/**
 * Will read all .atdf and .xml files in $path and attempt to parse them as AVR target description files.
 * This function is recursive - it will call itself if it stumbles upon a directory within $path.
 *
 * @param $path
 * @return TargetDescription[][]
 *  A mapping of target IDs to an array of TargetDescription objects. Note: target IDs are not
 *  always unique to targets. That is why each ID is mapped to an array of TargetDescription objects.
 */
function processAvrPartFiles($path) : array {
    global $processedFileCount, $failedFileCount;

    /** @var TargetDescription[][] $output */
    $output = [];

    foreach (glob($path . "/*") as $file) {
        if (is_dir($file)) {
            $output = array_merge($output, processAvrPartFiles($file));
            continue;
        }

        if (strstr($file, '.atdf') === false && strstr($file, '.xml') === false) {
            // Unknown file type
            continue;
        }

        $fileContents = file_get_contents($file);
        $pdXml = simplexml_load_string($fileContents);

        if ($pdXml === false) {
            print "Invalid XML in \"" . $file . "\"\n";
            $failedFileCount++;
            continue;
        }

        $device = $pdXml->devices[0]->device;

        $partDescriptionXml = new TargetDescription();
        $partDescriptionXml->originalFilePath = $file;
        $partDescriptionXml->destinationFilePath = TARGET_PD_DEST_FILE_PATH . "/";
        $partDescriptionXml->relativeDestinationFilePath = TARGET_PD_DEST_RELATIVE_FILE_PATH . "/";

        if (!empty($device['architecture'])
            && in_array($device['architecture'], ['AVR8', 'AVR32', 'AVR8X', 'AVR8L', 'AVR8_XMEGA'])
        ) {
            // This is an AVR device.
            if (!empty($device['architecture'])) {
                // Group by architecture
                $partDescriptionXml->destinationFilePath .= strtoupper((string) $device['architecture']) . "/";
                $partDescriptionXml->relativeDestinationFilePath .= strtoupper((string) $device['architecture']) . "/";
            }

            if (!empty($device['family'])) {
                // Group by family
                $partDescriptionXml->destinationFilePath .= str_replace([' '] , '_', strtoupper((string) $device['family']));
                $partDescriptionXml->relativeDestinationFilePath .= str_replace([' '] , '_', strtoupper((string) $device['family']));
            }

            $partDescriptionXml->targetName = str_replace([' '], '', (string) $device['name']);

            if (!empty(($signatures = $device->{'property-groups'}
                    ->xpath('property-group[@name="SIGNATURES"]')[0]))
                && !empty($signatures->xpath('property[@name="SIGNATURE0"]'))
                && !empty($signatures->xpath('property[@name="SIGNATURE1"]'))
                && !empty($signatures->xpath('property[@name="SIGNATURE2"]'))
            ) {
                $partDescriptionXml->targetId = (string) $signatures->xpath('property[@name="SIGNATURE0"]')[0]['value']
                    . str_replace('0x', '', (string) $signatures->xpath('property[@name="SIGNATURE1"]')[0]['value'])
                    . str_replace('0x', '', (string) $signatures->xpath('property[@name="SIGNATURE2"]')[0]['value'])
                ;
                $partDescriptionXml->targetId = strtolower($partDescriptionXml->targetId);
            }
        }

        if (empty($partDescriptionXml->destinationFilePath)
            || empty($partDescriptionXml->targetName)
            || empty($partDescriptionXml->targetId)
        ) {
            print "Failed to parse file {$file}\n";
            $failedFileCount++;
            continue;
        }

        if (!file_exists($partDescriptionXml->destinationFilePath)) {
            mkdir($partDescriptionXml->destinationFilePath, 0777, true);
        }

        $partDescriptionXml->destinationFilePath .= "/" . strtoupper($partDescriptionXml->targetName) . ".xml";
        $partDescriptionXml->relativeDestinationFilePath .= "/" . strtoupper($partDescriptionXml->targetName) . ".xml";

        if (strstr($fileContents, '<avr-tools-device-file ') !== false) {
            // Prefix auto gen comment
            // This approach could be better
            $fileContents = str_replace(
                '<avr-tools-device-file ',
                PD_COMMENT . "<avr-tools-device-file ",
                $fileContents
            );
        }

        if (file_put_contents($partDescriptionXml->destinationFilePath, $fileContents) === false) {
            print "FATAL ERROR: Failed to write data to " . $partDescriptionXml->destinationFilePath . "\n";
            print "Aborting\n";
            exit(1);
        }

        $output[$partDescriptionXml->targetId][] = $partDescriptionXml;
        echo "Target Part Description File Processed: \"" . substr($partDescriptionXml->originalFilePath, strlen(AVR_PD_FILE_PATH)) . "\"\n"
            . "Target Name: \"" . $partDescriptionXml->targetName . "\" Target ID: \"" . $partDescriptionXml->targetId
            . "\" Destination: \"" . substr($partDescriptionXml->destinationFilePath, strlen(TARGET_PD_DEST_FILE_PATH))
            . "\"\n\n"
        ;
        $processedFileCount++;
    }

    return $output;
}

print "Processing files in " . AVR_PD_FILE_PATH . "\n\n";
$targetDescriptions = processAvrPartFiles(AVR_PD_FILE_PATH);

if (file_put_contents(TARGET_PD_MAPPING_FILE_PATH, json_encode($targetDescriptions, JSON_PRETTY_PRINT)) === false) {
    print "FATAL ERROR: Failed to create JSON mapping of target IDs to target description file paths\n";
    exit(1);
}

print "\nCreated JSON mapping of target IDs to target description file paths: " . TARGET_PD_MAPPING_FILE_PATH . "\n";

print "\nProcessed " . $processedFileCount . " files. Failures: " . $failedFileCount . "\n";
print "Done\n";
