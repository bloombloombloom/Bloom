<?php

namespace Targets;

use Targets\TargetDescriptionFiles\TargetDescriptionFile;

require_once __DIR__ . "/TargetRegister.php";

/**
 * Do not confuse this with `TargetDescriptionFiles\RegisterGroup` - that class represents a <register-group> element
 * in a TDF. Any references to subgroups will be unresolved.
 *
 * This class represents a **resolved** target register group, within a target peripheral. With this class, we can
 * access all subgroups, including referenced subgroups, along with their registers.
 *
 * This class is constructed from a `TargetDescriptionFiles\RegisterGroup` object.
 * @see TargetDescriptionFile::getTargetRegisterGroup() for more.
 */
class TargetRegisterGroup
{
    public ?string $key = null;
    public ?string $absoluteKey = null;
    public ?string $name = null;
    public ?string $addressSpaceKey = null;
    public ?int $baseAddress = null;

    /** @var TargetRegisterGroup[] */
    public array $subgroups = [];

    /** @var TargetRegister[] */
    public array $registers = [];

    public function __construct(
        ?string $key,
        ?string $absoluteKey,
        ?string $name,
        ?string $addressSpaceKey,
        ?int $baseAddress,
        array $subgroups,
        array $registers
    ) {
        $this->key = $key;
        $this->absoluteKey = $absoluteKey;
        $this->name = $name;
        $this->addressSpaceKey = $addressSpaceKey;
        $this->baseAddress = $baseAddress;
        $this->subgroups = $subgroups;
        $this->registers = $registers;
    }

    public function getRegister(array|string $keys): ?TargetRegister
    {
        if (is_string($keys)) {
            $keys = explode('.', $keys);
        }

        $registerKey = array_pop($keys);
        $group = !empty($keys) > 1 ? $this->getSubgroup($keys) : $this;

        if ($group instanceof TargetRegisterGroup) {
            foreach ($group->registers as $register) {
                if ($register->key === $registerKey) {
                    return $register;
                }
            }
        }

        return null;
    }

    public function getSubgroup(array|string $subgroupKeys): ?TargetRegisterGroup
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
