<?php
namespace Targets\TargetDescriptionFiles;

class Signal
{
    public ?string $name = null;
    public ?string $padKey = null;
    public ?int $index = null;
    public ?string $function = null;
    public ?string $field = null;

    public function __construct(
        ?string $name,
        ?string $padKey,
        ?int $index,
        ?string $function,
        ?string $field
    ) {
        $this->name = $name;
        $this->padKey = $padKey;
        $this->index = $index;
        $this->function = $function;
        $this->field = $field;
    }
}
