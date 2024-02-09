<?php
namespace Targets\TargetDescriptionFiles;

class PhysicalInterface
{
    public ?string $name = null;
    public ?string $type = null;

    public function __construct(?string $name, ?string $type)
    {
        $this->name = $name;
        $this->type = $type;
    }
}
