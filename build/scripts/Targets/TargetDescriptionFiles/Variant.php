<?php
namespace Targets\TargetDescriptionFiles;

class Variant
{
    public ?string $name = null;
    public ?string $pinoutKey = null;
    public ?string $package = null;

    public function __construct(?string $name, ?string $pinoutKey, ?string $package)
    {
        $this->name = $name;
        $this->pinoutKey = $pinoutKey;
        $this->package = $package;
    }
}
