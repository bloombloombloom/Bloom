<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

require_once __DIR__ . "/Property.php";

class PropertyGroup
{
    public ?string $name = null;

    /** @var Property[] */
    public array $propertiesMappedByName = [];
}
