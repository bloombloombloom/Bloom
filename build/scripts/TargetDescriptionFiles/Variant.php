<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

class Variant
{
    public ?string $name = null;
    public ?string $package = null;
    public ?string $pinout = null;

    public function validate(): array
    {
        $failures = [];

        if (empty($this->name)) {
            $failures[] = 'Name not found';
        }

        if ($this->name == "standard") {
            $failures[] = 'Name set to "standard" - needs attention';
        }

        if (empty($this->package)) {
            $failures[] = 'Package not found';
        }

        return $failures;
    }
}
