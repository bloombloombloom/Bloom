<?php
namespace Targets\TargetDescriptionFiles\Avr8;

class BootSection
{
    public ?int $startAddress = null;
    public ?int $size = null;
    public ?int $pageSize = null;

    public function __construct(
        ?int $startAddress,
        ?int $size,
        ?int $pageSize
    ) {
        $this->startAddress = $startAddress;
        $this->size = $size;
        $this->pageSize = $pageSize;
    }
}
