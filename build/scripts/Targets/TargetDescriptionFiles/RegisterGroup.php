<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/Register.php";
require_once __DIR__ . "/RegisterGroupReference.php";

class RegisterGroup
{
    public ?string $key = null;
    public ?string $name = null;
    public ?int $offset = null;

    /** @var Register[] */
    public array $registersMappedByName = [];

    /** @var Register[] */
    public array $registers = [];

    /** @var RegisterGroup[] */
    public array $subGroups = [];

    /** @var RegisterGroupReference[] */
    public array $subGroupReferences = [];

    public function __construct(
        ?string $key,
        ?string $name,
        ?int $offset,
        array $registers,
        array $subGroups,
        array $subGroupReferences
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->offset = $offset;
        $this->registers = $registers;
        $this->subGroups = $subGroups;
        $this->subGroupReferences = $subGroupReferences;
    }

    public function getRegister(array|string $keys): ?Register
    {
        if (is_string($keys)) {
            $keys = explode('.', $keys);
        }

        $registerKey = array_pop($keys);
        $group = !empty($keys) > 1 ? $this->getSubGroup($keys) : $this;

        if ($group instanceof RegisterGroup) {
            foreach ($group->registers as $register) {
                if ($register->key === $registerKey) {
                    return $register;
                }
            }
        }

        return null;
    }

    public function getSubGroup(array|string $subGroupKeys): ?RegisterGroup
    {
        if (is_string($subGroupKeys)) {
            $subGroupKeys = explode('.', $subGroupKeys);
        }

        $firstLevelSubGroupKey = array_shift($subGroupKeys);
        foreach ($this->subGroups as $subGroup) {
            if ($subGroup->key === $firstLevelSubGroupKey) {
                return !empty($subGroupKeys) ? $subGroup->getSubGroup($subGroupKeys) : $subGroup;
            }
        }

        return null;
    }
}
