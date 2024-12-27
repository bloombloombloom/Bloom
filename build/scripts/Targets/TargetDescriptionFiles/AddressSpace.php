<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/AddressRange.php";
require_once __DIR__ . "/MemorySegment.php";

class AddressSpace
{
    public ?string $key = null;
    public ?AddressRange $addressRange = null;
    public ?int $unitSize = null;
    public ?string $endianness = null;

    /** @var MemorySegment[] */
    public array $memorySegments = [];

    public function __construct(?string $key, ?int $startAddress, ?int $size, ?int $unitSize, ?string $endianness)
    {
        $this->key = $key;
        $this->addressRange = is_numeric($startAddress) && is_numeric($size)
            ? new AddressRange(
                $startAddress,
                $startAddress + ($size / ($unitSize ?? 1)) - 1
            )
            : null;
        $this->unitSize = $unitSize;
        $this->endianness = $endianness;
    }

    public function size(): ?int
    {
        return $this->addressRange instanceof AddressRange
            ? $this->addressRange->size() * ($this->unitSize ?? 1)
            : null;
    }

    public function getMemorySegment(string $key): ?MemorySegment
    {
        foreach ($this->memorySegments as $segment) {
            if ($segment->key === $key) {
                return $segment;
            }
        }

        return null;
    }

    public function totalSegmentSize(): int
    {
        return array_sum(
            array_map(
                fn (MemorySegment $segment): int => (int) $segment->size(),
                $this->memorySegments
            )
        );
    }

    /**
     * Returns all memory segments that intercept with the given address range.
     *
     * @param int $startAddress
     * @param int $endAddress
     *
     * @return MemorySegment[]
     */
    public function findIntersectingMemorySegments(int $startAddress, int $endAddress): array
    {
        return array_filter(
            $this->memorySegments,
            function (MemorySegment $segment) use ($startAddress, $endAddress) : bool {
                $segmentEndAddress = $segment->addressRange->startAddress + $segment->size() - 1;
                return ($startAddress <= $segment->addressRange->startAddress && $endAddress >= $segment->addressRange->startAddress)
                    || ($startAddress >= $segment->addressRange->startAddress && $startAddress <= $segmentEndAddress);
            }
        );
    }
}
