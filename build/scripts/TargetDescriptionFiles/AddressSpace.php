<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

require_once __DIR__ . "/MemorySegment.php";

class AddressSpace
{
    public ?string $id = null;
    public ?string $name = null;
    public ?int $startAddress = null;
    public ?int $size = null;

    /** @var MemorySegment[][] */
    public array $memorySegmentsByTypeAndName = [];
}
