<?php
namespace Targets\TargetDescriptionFiles;

class Pin
{
    public ?string $position = null;
    public ?string $pad = null;

    public function __construct(?string $position, ?string $pad)
    {
        $this->position = $position;
        $this->pad = $pad;
    }
}
