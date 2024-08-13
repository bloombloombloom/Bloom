<?php
namespace Targets\TargetDescriptionFiles;

class Pad
{
    public ?string $key = null;
    public ?string $name = null;

    public function __construct(?string $key, ?string $name)
    {
        $this->key = $key;
        $this->name = $name;
    }
}
