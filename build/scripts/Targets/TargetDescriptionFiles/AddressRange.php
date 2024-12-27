<?php
namespace Targets\TargetDescriptionFiles;

class AddressRange
{
    public int $startAddress;
    public int $endAddress;

    public function __construct(int $startAddress, int $endAddress)
    {
        $this->startAddress = $startAddress;
        $this->endAddress = $endAddress;
    }

    public function size(): int
    {
        return $this->endAddress - $this->startAddress + 1;
    }

    public function contains(AddressRange $other): bool
    {
        return $this->startAddress <= $other->startAddress && $this->endAddress >= $other->endAddress;
    }

    public function intersectsWith(AddressRange $other): bool
    {
        return
            ($other->startAddress <= $this->startAddress && $other->endAddress >= $this->startAddress)
            || ($other->startAddress >= $this->startAddress && $other->startAddress <= $this->endAddress)
        ;
    }
}
