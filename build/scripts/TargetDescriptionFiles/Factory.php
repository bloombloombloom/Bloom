<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

use Bloom\BuildScripts\TargetDescriptionFiles\Avr8\Avr8TargetDescriptionFile;

require_once __DIR__ . "/TargetDescriptionFile.php";
require_once __DIR__ . "/TargetFamily.php";

require_once __DIR__ . "/AVR8/Avr8TargetDescriptionFile.php";

class Factory
{
    /**
     * Loads a target description file with the appropriate class.
     *
     * @param string $filePath
     * @return TargetDescriptionFile
     */
    public static function loadTdfFromFile(string $filePath): TargetDescriptionFile
    {
        $tdf = new TargetDescriptionFile($filePath);

        if ($tdf->targetFamily == TargetFamily::AVR_8) {
            return new Avr8TargetDescriptionFile($filePath);
        }

        return $tdf;
    }

    /**
     * Recursively finds all XML files within a given directory.
     *
     * @param string $dirPath
     * @return \SplFileInfo[]
     */
    public static function findXmlFiles(string $dirPath): array
    {
        $output = [];

        $directory = new \DirectoryIterator($dirPath);
        foreach ($directory as $entry) {
            if ($entry->isFile() && $entry->getExtension() == 'xml') {
                $output[] = clone $entry;

            } else if ($entry->isDir() && !$entry->isDot()) {
                $output = array_merge($output, self::findXmlFiles($entry->getPathname()));
            }
        }

        return $output;
    }
}
