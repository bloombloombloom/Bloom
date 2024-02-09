<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/RegisterGroup.php";
require_once __DIR__ . "/RegisterGroupReference.php";
require_once __DIR__ . "/Signal.php";

class Peripheral
{
    public ?string $key = null;
    public ?string $name = null;
    public ?string $moduleKey = null;
    public ?string $moduleName = null;

    /** @var RegisterGroupReference[] */
    public array $registerGroupReferences = [];

    /** @var Signal[] */
    public array $signals = [];

    public function __construct(
        ?string $key,
        ?string $name,
        ?string $moduleKey,
        array $registerGroupReferences,
        array $signals
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->moduleKey = $moduleKey;
        $this->registerGroupReferences = $registerGroupReferences;
        $this->signals = $signals;
    }

    public function getRegisterGroupReference(string $key): ?RegisterGroupReference
    {
        foreach ($this->registerGroupReferences as $reference) {
            if ($reference->key === $key) {
                return $reference;
            }
        }

        return null;
    }
}
