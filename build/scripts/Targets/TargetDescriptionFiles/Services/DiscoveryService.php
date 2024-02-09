<?php
namespace Targets\TargetDescriptionFiles\Services;

class DiscoveryService
{
    /**
     * Searches for TDFs within the given directory.
     *
     * This is a fairly rudimentary implementation - we just assume all XML files are TDFs. This should be sufficient,
     * as TDFs typically reside in dedicated directories.
     *
     * @param string $directoryPath
     *
     * @return \SplFileInfo[]
     *  An SplFileInfo instance for each TDF file.
     */
    public function findTargetDescriptionFiles(string $directoryPath): array
    {
        $output = [];

        $directory = new \DirectoryIterator($directoryPath);
        foreach ($directory as $entry) {
            if ($entry->isFile() && $entry->getExtension() == 'xml') {
                $output[] = clone $entry;

            } elseif ($entry->isDir() && !$entry->isDot()) {
                $output = array_merge($output, $this->findTargetDescriptionFiles($entry->getPathname()));
            }
        }

        return $output;
    }
}
