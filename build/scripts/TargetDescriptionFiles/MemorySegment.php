<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

class MemorySegment
{
    public ?string $name = null;
    public ?string $type = null;
    public ?int $startAddress = null;
    public ?int $size = null;
    public ?int $pageSize = null;
}
