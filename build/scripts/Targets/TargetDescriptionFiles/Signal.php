<?php
namespace Targets\TargetDescriptionFiles;

class Signal
{
    public ?string $padKey = null;
    public ?int $index = null;
    public ?string $function = null;
    public ?string $group = null;
    public ?string $field = null;

    public function __construct(
        ?string $padKey,
        ?int $index,
        ?string $function,
        ?string $group,
        ?string $field
    ) {
        $this->padKey = $padKey;
        $this->index = $index;
        $this->function = $function;
        $this->group = $group;
        $this->field = $field;
    }
}
