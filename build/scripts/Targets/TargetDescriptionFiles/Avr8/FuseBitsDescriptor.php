<?php
namespace Targets\TargetDescriptionFiles\Avr8;

class FuseBitsDescriptor
{
    public ?string $fuseType = null;

    public function __construct(?string $fuseType)
    {
        $this->fuseType = strtolower($fuseType);
    }
}
