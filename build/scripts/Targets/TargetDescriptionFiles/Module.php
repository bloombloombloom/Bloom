<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/RegisterGroup.php";

class Module
{
    public ?string $key = null;
    public ?string $name = null;
    public ?string $description = null;

    /** @var RegisterGroup[] */
    public array $registerGroups = [];

    public function __construct(
        ?string $key,
        ?string $name,
        ?string $description,
        array $registerGroups,
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->description = $description;
        $this->registerGroups = $registerGroups;
    }

    public function getRegisterGroup(array|string $keys): ?RegisterGroup
    {
        if (is_string($keys)) {
            $keys = explode('.', $keys);
        }

        $firstLevelSubGroupId = array_shift($keys);
        foreach ($this->registerGroups as $registerGroup) {
            if ($registerGroup->key === $firstLevelSubGroupId) {
                return !empty($keys) ? $registerGroup->getSubGroup($keys) : $registerGroup;
            }
        }

        return null;
    }
}
