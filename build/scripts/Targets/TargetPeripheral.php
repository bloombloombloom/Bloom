<?php
namespace Targets;

use Targets\TargetDescriptionFiles\TargetDescriptionFile;

require_once __DIR__ . "/TargetRegisterGroup.php";

/**
 * Do not confuse this with `TargetDescriptionFiles\Peripheral` - that class represents a <peripheral> element in a
 * TDF, meaning it only contains references to register groups and their respective offsets. Those register group
 * references are unresolved, meaning you can't access any of the referenced group's registers or subgroups.
 *
 * This class represents a **resolved** target peripheral - meaning all the referenced register groups are resolved,
 * along with their respective addresses. With this class, we can access all peripheral registers/register groups, and
 * their absolute addresses.
 *
 * This class is constructed from a `TargetDescriptionFiles\Peripheral` object.
 * @see TargetDescriptionFile::getTargetPeripheral() for more.
 */
class TargetPeripheral
{
    public ?string $name = null;

    /** @var TargetRegisterGroup[] */
    public array $registerGroups = [];

    public function __construct(?string $name, array $registerGroups)
    {
        $this->name = $name;
        $this->registerGroups = $registerGroups;
    }

    public function getRegisterGroup(array|string $keys): ?TargetRegisterGroup
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

    public function getRegister(array|string $groupKeys, array|string $keys): ?TargetRegister
    {
        return ($group = $this->getRegisterGroup($groupKeys)) instanceof TargetRegisterGroup
            ? ($register = $group->getRegister($keys)) instanceof TargetRegister ? $register : null
            : null;
    }
}
