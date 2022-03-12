<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

require_once __DIR__ . "/BitField.php";

class Register
{
    public ?string $name = null;
    public ?int $offset = null;
    public ?int $size = null;

    /** @var BitField[] */
    public array $bitFieldsByName = [];
}
