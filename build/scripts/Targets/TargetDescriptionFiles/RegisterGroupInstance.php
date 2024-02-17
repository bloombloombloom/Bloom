<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/Register.php";

class RegisterGroupInstance
{
    public ?string $key = null;
    public ?string $name = null;
    public ?string $registerGroupKey = null;
    public ?string $addressSpaceKey = null;
    public ?int $offset = null;
    public ?string $description = null;

    public function __construct(
        ?string $key,
        ?string $name,
        ?string $registerGroupKey,
        ?string $addressSpaceKey,
        ?int $offset,
        ?string $description
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->registerGroupKey = $registerGroupKey;
        $this->addressSpaceKey = $addressSpaceKey;
        $this->offset = $offset;
        $this->description = $description;
    }
}
