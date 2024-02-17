<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/RegisterGroup.php";
require_once __DIR__ . "/RegisterGroupInstance.php";
require_once __DIR__ . "/Signal.php";

class Peripheral
{
    public ?string $key = null;
    public ?string $name = null;
    public ?string $moduleKey = null;
    public ?string $moduleName = null;

    /** @var RegisterGroupInstance[] */
    public array $registerGroupInstances = [];

    /** @var Signal[] */
    public array $signals = [];

    public function __construct(
        ?string $key,
        ?string $name,
        ?string $moduleKey,
        array $registerGroupInstances,
        array $signals
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->moduleKey = $moduleKey;
        $this->registerGroupInstances = $registerGroupInstances;
        $this->signals = $signals;
    }

    public function getRegisterGroupInstance(string $key): ?RegisterGroupInstance
    {
        foreach ($this->registerGroupInstances as $instance) {
            if ($instance->key === $key) {
                return $instance;
            }
        }

        return null;
    }
}
