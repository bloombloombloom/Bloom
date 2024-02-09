<?php
namespace Targets\TargetDescriptionFiles;

class Signal
{
    public ?string $padId = null;
    public ?int $index = null;
    public ?string $function = null;
    public ?string $group = null;
    public ?string $field = null;

    public function __construct(
        ?string $padId,
        ?int $index,
        ?string $function,
        ?string $group,
        ?string $field
    ) {
        $this->padId = $padId;
        $this->index = $index;
        $this->function = $function;
        $this->group = $group;
        $this->field = $field;
    }
}
