<?php
namespace Targets\TargetDescriptionFiles;

class Variant
{
    public ?string $key = null;
    public ?string $name = null;
    public ?string $pinoutKey = null;

    public function __construct(?string $key, ?string $name, ?string $pinoutKey)
    {
        $this->key = $key;
        $this->name = $name;
        $this->pinoutKey = $pinoutKey;
    }
}
