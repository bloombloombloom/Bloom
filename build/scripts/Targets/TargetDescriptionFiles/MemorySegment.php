<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/AddressRange.php";
require_once __DIR__ . "/MemorySegmentType.php";
require_once __DIR__ . "/MemorySegmentSection.php";

class MemorySegment
{
    public ?string $key = null;
    public ?string $name = null;
    public ?MemorySegmentType $type = null;
    public ?AddressRange $addressRange = null;
    public ?int $addressSpaceUnitSize = null;
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
        ?int $addressSpaceUnitSize,
        ?int $pageSize,
        ?string $access,
        array $sections,
        ?bool $executable
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->type = $type;
        $this->addressRange = is_numeric($startAddress) && is_numeric($size)
            ? new AddressRange(
                $startAddress,
                $startAddress + ($size / ($addressSpaceUnitSize ?? 1)) - 1
            )
            : null;
        $this->addressSpaceUnitSize = $addressSpaceUnitSize;
        $this->pageSize = $pageSize;
        $this->access = $access;
        $this->sections = $sections;
        $this->executable = $executable;
    }

    public function size(): ?int
    {
        return $this->addressRange instanceof AddressRange
            ? $this->addressRange->size() * ($this->addressSpaceUnitSize ?? 1)
            : null;
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

    public function getInnermostSectionContainingAddressRange(AddressRange $range)
    : ?MemorySegmentSection {
        foreach ($this->sections as $section) {
            if ($section->containsAddressRange($range)) {
                return $section->getInnermostSubSectionContainingAddressRange($range);
            }
        }

        return null;
    }

    public function contains(MemorySegment $other): bool
    {
        return $this->addressRange instanceof AddressRange && $this->addressRange->contains($other->addressRange);
    }

    public function intersectsWith(MemorySegment $other): bool
    {
        return
            $this->addressRange instanceof AddressRange
            && $other->addressRange instanceof AddressRange
            && $this->addressRange->intersectsWith($other->addressRange)
        ;
    }

    public function totalSectionSize(): int
    {
        return array_sum(
            array_map(
                fn (MemorySegmentSection $section): int => (int) $section->size(),
                $this->sections
            )
        );
    }
}
