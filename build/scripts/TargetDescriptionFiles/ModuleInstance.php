<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

require_once __DIR__ . "/RegisterGroup.php";
require_once __DIR__ . "/Signal.php";

class ModuleInstance
{
    public ?string $name = null;

    /** @var RegisterGroup[] */
    public array $registerGroupsMappedByName = [];

    /** @var Signal[]  */
    public array $signals = [];
}
