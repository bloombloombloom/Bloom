<?php
namespace Targets\TargetDescriptionFiles;

class MemorySegmentSection
{
    public ?string $key = null;
    public ?string $name = null;
    public ?int $startAddress = null;
    public ?int $size = null;

    /** @var MemorySegmentSection[] */
    public array $subSections = [];

    public function __construct(
        ?string $key,
        ?string $name,
        ?int $startAddress,
        ?int $size,
        array $subSections
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->startAddress = $startAddress;
        $this->size = $size;
        $this->subSections = $subSections;
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

    public function intersectsWith(MemorySegmentSection $other): bool
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
}
