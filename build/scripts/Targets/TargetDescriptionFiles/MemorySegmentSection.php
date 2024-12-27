<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/AddressRange.php";

class MemorySegmentSection
{
    public ?string $key = null;
    public ?string $name = null;
    public ?AddressRange $addressRange = null;
    public ?int $addressSpaceUnitSize = null;

    /** @var MemorySegmentSection[] */
    public array $subSections = [];

    public function __construct(
        ?string $key,
        ?string $name,
        ?int $startAddress,
        ?int $size,
        ?int $addressSpaceUnitSize,
        array $subSections
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->addressRange = is_numeric($startAddress) && is_numeric($size)
            ? new AddressRange(
                $startAddress,
                $startAddress + ($size / ($addressSpaceUnitSize ?? 1)) - 1
            )
            : null;
        $this->addressSpaceUnitSize = $addressSpaceUnitSize;
        $this->subSections = $subSections;
    }

    public function size(): ?int
    {
        return $this->addressRange instanceof AddressRange
            ? $this->addressRange->size() * ($this->addressSpaceUnitSize ?? 1)
            : null;
    }

    public function getInnermostSubSectionContainingAddressRange(AddressRange $range)
    : ?MemorySegmentSection {
        if (!$this->containsAddressRange($range)) {
            return null;
        }

        foreach ($this->subSections as $section) {
            if ($section->containsAddressRange($range)) {
                return $section->getInnermostSubSectionContainingAddressRange($range);
            }
        }

        return $this;
    }

    public function contains(MemorySegmentSection $other): bool
    {
        return $this->addressRange instanceof AddressRange && $this->addressRange->contains($other->addressRange);
    }

    public function containsAddressRange(AddressRange $range): bool
    {
        return $this->addressRange instanceof AddressRange && $this->addressRange->contains($range);
    }

    public function intersectsWith(MemorySegmentSection $other): bool
    {
        return
            $this->addressRange instanceof AddressRange
            && $other->addressRange instanceof AddressRange
            && $this->addressRange->intersectsWith($other->addressRange)
        ;
    }
}
