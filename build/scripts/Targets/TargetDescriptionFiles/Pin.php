<?php
namespace Targets\TargetDescriptionFiles;

class Pin
{
    public ?string $position = null;
    public ?string $padKey = null;

    public function __construct(?string $position, ?string $padKey)
    {
        $this->position = $position;
        $this->padKey = $padKey;
    }
}
