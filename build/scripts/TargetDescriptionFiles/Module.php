<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

require_once __DIR__ . "/RegisterGroup.php";
require_once __DIR__ . "/ModuleInstance.php";

class Module
{
    public ?string $name = null;

    /** @var RegisterGroup[] */
    public array $registerGroupsMappedByName = [];

    /** @var ModuleInstance[] */
    public array $instancesMappedByName = [];
}
