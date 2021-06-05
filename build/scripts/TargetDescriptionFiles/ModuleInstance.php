<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

require_once __DIR__ . "/RegisterGroup.php";

class ModuleInstance
{
    public ?string $name = null;

    /** @var RegisterGroup[] */
    public array $registerGroupsMappedByName = [];
}
