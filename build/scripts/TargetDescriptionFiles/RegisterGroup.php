<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

require_once __DIR__ . "/Register.php";

class RegisterGroup
{
    public ?string $name = null;
    public ?int $offset = null;

    /** @var Register[] */
    public array $registersMappedByName = [];
}
