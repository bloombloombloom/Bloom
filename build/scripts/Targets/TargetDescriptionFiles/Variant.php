<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/PropertyGroup.php";

class Variant
{
    public ?string $key = null;
    public ?string $name = null;
    public ?string $pinoutKey = null;

    /** @var PropertyGroup[] */
    public array $propertyGroups = [];

    public function __construct(?string $key, ?string $name, ?string $pinoutKey, array $propertyGroups)
    {
        $this->key = $key;
        $this->name = $name;
        $this->pinoutKey = $pinoutKey;
        $this->propertyGroups = $propertyGroups;
    }

    public function getPropertyGroup(array|string $keys): ?PropertyGroup
    {
        if (is_string($keys)) {
            $keys = explode('.', $keys);
        }

        $firstLevelGroupKey = array_shift($keys);
        foreach ($this->propertyGroups as $propertyGroup) {
            if ($propertyGroup->key === $firstLevelGroupKey) {
                return !empty($keys) ? $propertyGroup->getSubgroup($keys) : $propertyGroup;
            }
        }

        return null;
    }

    public function getProperty(array|string $propertyGroupKeys, $propertyKey): ?Property
    {
        return ($propertyGroup = $this->getPropertyGroup($propertyGroupKeys)) instanceof PropertyGroup
            ? $propertyGroup->getProperty($propertyKey)
            : null;
    }

    public function getPropertyValue(array|string $propertyGroupKeys, $propertyKey): ?string
    {
        return ($property = $this->getProperty($propertyGroupKeys, $propertyKey)) instanceof Property
            ? $property->value
            : null;
    }
}
