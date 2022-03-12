<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

class FuseBitDescriptor
{
    const FUSE_TYPE_LOW = 'low';
    const FUSE_TYPE_HIGH = 'high';
    const FUSE_TYPE_EXTENDED = 'extended';

    public ?string $fuseType = null;
}
