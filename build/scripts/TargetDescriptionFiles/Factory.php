<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

use Bloom\BuildScripts\TargetDescriptionFiles\Avr8\Avr8TargetDescriptionFile;

require_once __DIR__ . "/TargetDescriptionFile.php";
require_once __DIR__ . "/AVR8/Avr8TargetDescriptionFile.php";

class Factory
{
    private const TDF_PATH = __DIR__ . '/../../../src/Targets/TargetDescriptionFiles';
    private const AVR8_TDF_PATH = self::TDF_PATH . '/AVR8';

    /**
     * Loads a target description file with the appropriate class.
     *
     * @param string $filePath
     * @return TargetDescriptionFile
     */
    public static function loadTdfFromFile(string $filePath): TargetDescriptionFile
    {
        $tdf = new TargetDescriptionFile($filePath);

        if ($tdf->targetArchitecture == TargetDescriptionFile::ARCHITECTURE_AVR8) {
            $tdf = new Avr8TargetDescriptionFile($filePath);
        }

        return $tdf;
    }

    /**
     * Loads all AVR8 target description files.
     *
     * @return Avr8TargetDescriptionFile[]
     */
    public static function loadAvr8Tdfs(): array
    {
        /** @var Avr8TargetDescriptionFile[] $output */
        $output = [];

        foreach (self::loadXmlFiles(self::AVR8_TDF_PATH) as $xmlFile) {
            $output[] = new Avr8TargetDescriptionFile($xmlFile->getPathname());
        }

        return $output;
    }

    /**
     * Recursively loads all XML files from a given directory.
     *
     * @param string $dirPath
     * @return \SplFileInfo[]
     */
    private static function loadXmlFiles(string $dirPath): array
    {
        $output = [];

        $directory = new \DirectoryIterator($dirPath);
        foreach ($directory as $entry) {
            if ($entry->isFile() && $entry->getExtension() == 'xml') {
                $output[] = clone $entry;

            } else if ($entry->isDir() && !$entry->isDot()) {
                $output = array_merge($output, self::loadXmlFiles($entry->getPathname()));
            }
        }

        return $output;
    }
}
