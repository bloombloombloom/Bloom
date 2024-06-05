<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/MemorySegment.php";

class AddressSpace
{
    public ?string $key = null;
    public ?int $startAddress = null;
    public ?int $size = null;
    public ?string $endianness = null;

    /** @var MemorySegment[] */
    public array $memorySegments = [];

    public function __construct(?string $key, ?int $startAddress, ?int $size, ?string $endianness)
    {
        $this->key = $key;
        $this->startAddress = $startAddress;
        $this->size = $size;
        $this->endianness = $endianness;
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
                fn (MemorySegment $segment): int => (int) $segment->size,
                $this->memorySegments
            )
        );
    }

    public function segmentStartAddress(): int
    {
        return min(
            array_map(
                fn (MemorySegment $segment): int => (int) $segment->startAddress,
                $this->memorySegments
            )
        );
    }

    public function segmentEndAddress(): int
    {
        return max(
            array_map(
                fn (MemorySegment $segment): int => (int) $segment->startAddress + $segment->size - 1,
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
                $segmentEndAddress = $segment->startAddress + $segment->size - 1;
                return ($startAddress <= $segment->startAddress && $endAddress >= $segment->startAddress)
                    || ($startAddress >= $segment->startAddress && $startAddress <= $segmentEndAddress);
            }
        );
    }
}
