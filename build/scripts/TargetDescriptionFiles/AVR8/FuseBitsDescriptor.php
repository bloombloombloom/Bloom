<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

class FuseBitsDescriptor
{
    const FUSE_TYPE_LOW = 'low';
    const FUSE_TYPE_HIGH = 'high';
    const FUSE_TYPE_EXTENDED = 'extended';

    public ?string $fuseType = null;

    public function __construct(?string $fuseType)
    {
        $this->fuseType = $fuseType;
    }
}
