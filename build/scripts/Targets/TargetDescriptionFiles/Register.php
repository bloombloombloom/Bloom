<?php
namespace Targets\TargetDescriptionFiles;

require_once __DIR__ . "/BitField.php";

class Register
{
    public ?string $key = null;
    public ?string $name = null;
    public ?string $description = null;
    public ?int $offset = null;
    public ?int $size = null;
    public ?int $initialValue = null;
    public ?string $access = null;
    public ?bool $alternative = null;

    /** @var BitField[] */
    public array $bitFieldsByName = [];

    /** @var BitField[] */
    public array $bitFields = [];

    public function __construct(
        ?string $key,
        ?string $name,
        ?string $description,
        ?int $offset,
        ?int $size,
        ?int $initialValue,
        ?string $access,
        ?bool $alternative,
        array $bitFields
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->description = $description;
        $this->offset = $offset;
        $this->size = $size;
        $this->initialValue = $initialValue;
        $this->access = $access;
        $this->alternative = $alternative;
        $this->bitFields = $bitFields;
    }

    public function intersectsWith(Register $other): bool
    {
        $endAddress = !is_null($this->offset) && !is_null($this->size)
            ? ($this->offset + $this->size - 1) : null;
        $otherEndAddress = !is_null($other->offset) && !is_null($other->size)
            ? ($other->offset + $other->size - 1) : null;

        return
            $this->offset !== null
            && $endAddress !== null
            && $other->offset !== null
            && $otherEndAddress !== null
            && (
                ($other->offset <= $this->offset && $otherEndAddress >= $this->offset)
                || ($other->offset >= $this->offset && $other->offset <= $endAddress)
            )
        ;
    }
}
