<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

require_once __DIR__ . "/Pin.php";

class Pinout
{
    const TYPE_SOIC = 'SOIC';
    const TYPE_DIP = 'DIP';
    const TYPE_QFN = 'QFN';
    const TYPE_BGA = 'BGA';

    public ?string $name = null;
    public ?string $function = null;
    public ?string $type = null;

    /**
     * The name of the pinouts typically include the number of pins. We extract this if we can, so that we can
     * validate against it.
     *
     * For example, for an SOIC pinout with 20 pins, the name of the pinout would be something like "SOIC20"
     * or "SOIC_20".
     *
     * @var int|null
     */
    public ?int $expectedPinCount = null;

    /** @var Pin[] */
    public array $pins = [];
}
