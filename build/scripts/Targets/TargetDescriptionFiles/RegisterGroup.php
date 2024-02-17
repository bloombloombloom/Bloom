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
    public array $registers = [];

    /** @var RegisterGroup[] */
    public array $subgroups = [];

    /** @var RegisterGroupReference[] */
    public array $subgroupReferences = [];

    public function __construct(
        ?string $key,
        ?string $name,
        ?int $offset,
        array $registers,
        array $subgroups,
        array $subgroupReferences
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->offset = $offset;
        $this->registers = $registers;
        $this->subgroups = $subgroups;
        $this->subgroupReferences = $subgroupReferences;
    }

    public function getRegister(array|string $keys): ?Register
    {
        if (is_string($keys)) {
            $keys = explode('.', $keys);
        }

        $registerKey = array_pop($keys);
        $group = !empty($keys) > 1 ? $this->getSubgroup($keys) : $this;

        if ($group instanceof RegisterGroup) {
            foreach ($group->registers as $register) {
                if ($register->key === $registerKey) {
                    return $register;
                }
            }
        }

        return null;
    }

    public function getSubgroup(array|string $subgroupKeys): ?RegisterGroup
    {
        if (is_string($subgroupKeys)) {
            $subgroupKeys = explode('.', $subgroupKeys);
        }

        $firstLevelSubgroupKey = array_shift($subgroupKeys);
        foreach ($this->subgroups as $subgroup) {
            if ($subgroup->key === $firstLevelSubgroupKey) {
                return !empty($subgroupKeys) ? $subgroup->getSubgroup($subgroupKeys) : $subgroup;
            }
        }

        return null;
    }
}
