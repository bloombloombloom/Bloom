<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/PinoutType.php";
require_once __DIR__ . "/Pin.php";

class Pinout
{
    public ?string $key = null;
    public ?string $name = null;
    public ?PinoutType $type = null;
    public ?string $function = null;

    /** @var Pin[] */
    public array $pins = [];

    public function __construct(
        ?string $key,
        ?string $name,
        ?PinoutType $type,
        ?string $function,
        array $pins
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->type = $type;
        $this->function = $function;
        $this->pins = $pins;
    }
}
