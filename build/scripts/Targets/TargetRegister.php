<?php
namespace Targets;

use Targets\TargetDescriptionFiles\TargetDescriptionFile;

require_once __DIR__ . "/TargetRegisterBitField.php";

/**
 * Do not confuse this with `TargetDescriptionFiles\Register` - that class represents a <register> element in a TDF,
 * which is part of some unresolved register group.
 *
 * This class represents a **resolved** target register, within a register group, within a target peripheral. With
 * this class, we can access the absolute address of the register.
 *
 * This class is constructed from a `TargetDescriptionFiles\Register` object.
 * @see TargetDescriptionFile::getTargetRegister() for more.
 */
class TargetRegister
{
    public ?string $key = null;
    public ?string $name = null;
    public ?int $address = null;
    public ?int $size = null;
    public ?string $description = null;

    /** @var TargetRegisterBitField[] */
    public array $bitFields;

    public function __construct(
        ?string $key,
        ?string $name,
        ?int $address,
        ?int $size,
        ?string $description,
        array $bitFields
    ) {
        $this->key = $key;
        $this->name = $name;
        $this->address = $address;
        $this->size = $size;
        $this->description = $description;
        $this->bitFields = $bitFields;
    }
}
