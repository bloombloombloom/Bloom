<?php
namespace Targets\TargetDescriptionFiles;

class PhysicalInterface
{
    public ?string $value = null;

    /** @var Signal[] */
    public array $signals = [];

    public function __construct(?string $value, array $signals)
    {
        $this->value = $value;
        $this->signals = $signals;
    }
}
