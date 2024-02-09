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
    public ?string $name = null;
    public ?int $baseAddress = null;

    /** @var TargetRegisterGroup[] */
    public array $subGroups = [];

    /** @var TargetRegister[] */
    public array $registers = [];

    public function __construct(?string $key, ?string $name, ?int $baseAddress, array $subGroups, array $registers)
    {
        $this->key = $key;
        $this->name = $name;
        $this->baseAddress = $baseAddress;
        $this->subGroups = $subGroups;
        $this->registers = $registers;
    }

    public function getRegister(array|string $keys): ?TargetRegister
    {
        if (is_string($keys)) {
            $keys = explode('.', $keys);
        }

        $registerKey = array_pop($keys);
        $group = !empty($keys) > 1 ? $this->getSubGroup($keys) : $this;

        if ($group instanceof TargetRegisterGroup) {
            foreach ($group->registers as $register) {
                if ($register->key === $registerKey) {
                    return $register;
                }
            }
        }

        return null;
    }

    public function getSubGroup(array|string $subGroupKeys): ?TargetRegisterGroup
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
