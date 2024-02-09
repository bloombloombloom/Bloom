<?php
namespace Targets\TargetDescriptionFiles;

class Property
{
    public ?string $key = null;
    public ?string $value = null;

    public function __construct(?string $key, ?string $value)
    {
        $this->key = $key;
        $this->value = $value;
    }
}
