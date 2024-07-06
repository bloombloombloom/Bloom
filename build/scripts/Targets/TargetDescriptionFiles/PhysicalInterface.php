<?php
namespace Targets\TargetDescriptionFiles;

class PhysicalInterface
{
    public ?string $value = null;

    public function __construct(?string $value)
    {
        $this->value = $value;
    }
}
