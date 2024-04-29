<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/MemorySegmentType.php";
require_once __DIR__ . "/MemorySegmentSection.php";

class MemorySegment
{
    public ?string $key = null;
    public ?string $name = null;
    public ?MemorySegmentType $type = null;
    public ?int $startAddress = null;
    public ?int $size = null;
    public ?int $pageSize = null;
    public ?string $access = null;
    public ?bool $executable = null;

    /** @var MemorySegmentSection[] */
    public array $sections = [];

    public function __construct(
        ?string $key,
        ?string $name,
        ?MemorySegmentType $type,
        ?int $startAddress,
        ?int $size,
        ?int $pageSize,
        ?string $access,
        array $sections,
        ?bool $executable
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->type = $type;
        $this->startAddress = $startAddress;
        $this->size = $size;
        $this->pageSize = $pageSize;
        $this->access = $access;
        $this->sections = $sections;
        $this->executable = $executable;
    }

    public function getSection(string $sectionId): ?MemorySegmentSection
    {
        foreach ($this->sections as $section) {
            if ($section->key !== $sectionId) {
                continue;
            }

            return $section;
        }

        return null;
    }

    public function getInnermostSectionContainingAddressRange(int $startAddress, int $endAddress)
    : ?MemorySegmentSection {
        foreach ($this->sections as $section) {
            if ($section->containsAddressRange($startAddress, $endAddress)) {
                return $section->getInnermostSubSectionContainingAddressRange($startAddress, $endAddress);
            }
        }

        return null;
    }

    public function contains(MemorySegment $other): bool
    {
        $endAddress = !is_null($this->startAddress) && !is_null($this->size)
            ? ($this->startAddress + $this->size - 1) : null;
        $otherEndAddress = !is_null($other->startAddress) && !is_null($other->size)
            ? ($other->startAddress + $other->size - 1) : null;

        return
            $this->startAddress !== null
            && $endAddress !== null
            && $other->startAddress !== null
            && $otherEndAddress !== null
            && $this->startAddress <= $other->startAddress
            && $endAddress >= $otherEndAddress
        ;
    }

    public function intersectsWith(MemorySegment $other): bool
    {
        $endAddress = !is_null($this->startAddress) && !is_null($this->size)
            ? ($this->startAddress + $this->size - 1) : null;
        $otherEndAddress = !is_null($other->startAddress) && !is_null($other->size)
            ? ($other->startAddress + $other->size - 1) : null;

        return
            $this->startAddress !== null
            && $endAddress !== null
            && $other->startAddress !== null
            && $otherEndAddress !== null
            && (
                ($other->startAddress <= $this->startAddress && $otherEndAddress >= $this->startAddress)
                || ($other->startAddress >= $this->startAddress && $other->startAddress <= $endAddress)
            )
        ;
    }

    public function totalSectionSize(): int
    {
        return array_sum(
            array_map(
                fn (MemorySegmentSection $section): int => (int) $section->size,
                $this->sections
            )
        );
    }
}
