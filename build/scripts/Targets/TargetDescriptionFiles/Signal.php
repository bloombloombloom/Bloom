<?php
namespace Targets\TargetDescriptionFiles;

class Signal
{
    public ?string $name = null;
    public ?string $padKey = null;
    public ?bool $alternative = null;
    public ?int $index = null;
    public ?string $function = null;
    public ?string $field = null;

    public function __construct(
        ?string $name,
        ?string $padKey,
        ?bool $alternative,
        ?int $index,
        ?string $function,
        ?string $field
    ) {
        $this->name = $name;
        $this->padKey = $padKey;
        $this->alternative = $alternative;
        $this->index = $index;
        $this->function = $function;
        $this->field = $field;
    }
}
