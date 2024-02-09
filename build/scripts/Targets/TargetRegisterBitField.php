<?php
namespace Targets;

class TargetRegisterBitField
{
    public ?string $key = null;
    public ?string $name = null;
    public ?string $description = null;
    public ?int $mask = null;
    public ?string $access = null;

    public function __construct(
        ?string $key,
        ?string $name,
        ?string $description,
        ?int $mask,
        ?string $access
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->description = $description;
        $this->mask = $mask;
        $this->access = $access;
    }
}
