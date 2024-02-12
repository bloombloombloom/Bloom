<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/Property.php";

class PropertyGroup
{
    public ?string $key = null;

    /** @var Property[] */
    public array $properties = [];

    /** @var PropertyGroup[] */
    public array $subPropertyGroups = [];

    public function __construct(
        ?string $key,
        array $subPropertyGroups,
        array $properties,
    ) {
        $this->key = $key;
        $this->subPropertyGroups = $subPropertyGroups;
        $this->properties = $properties;
    }

    public function getSubgroup(array|string $subgroupKeys): ?PropertyGroup
    {
        if (is_string($subgroupKeys)) {
            $subgroupKeys = explode('.', $subgroupKeys);
        }

        $firstLevelSubgroupKey = array_shift($subgroupKeys);
        foreach ($this->subPropertyGroups as $subgroup) {
            if ($subgroup->key === $firstLevelSubgroupKey) {
                return !empty($subgroupKeys) ? $subgroup->getSubgroup($subgroupKeys) : $subgroup;
            }
        }

        return null;
    }

    public function getProperty(string $propertyKey): ?Property
    {
        foreach ($this->properties as $property) {
            if ($property->key === $propertyKey) {
                return $property;
            }
        }

        return null;
    }

    public function getPropertyValue(string $propertyKey): ?string
    {
        return ($property = $this->getProperty($propertyKey)) instanceof Property ? $property->value : null;
    }
}
