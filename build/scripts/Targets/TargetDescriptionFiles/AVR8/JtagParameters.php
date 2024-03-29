<?php
namespace Targets\TargetDescriptionFiles\Avr8;

require_once __DIR__ . '/BootSection.php';

class JtagParameters
{
    public ?int $flashPageSize = null;
    public ?int $flashSize = null;
    public ?int $flashStartAddress = null;

    /** @var BootSection[]  */
    public array $bootSectionOptions = [];

    public ?int $ramStartAddress = null;
    public ?int $eepromSize = null;
    public ?int $eepromPageSize = null;
    public ?int $ocdRevision = null;
    public ?int $ocdDataRegisterAddress = null;
    public ?int $eearAddressHigh = null;
    public ?int $eearAddressLow = null;
    public ?int $eecrAddress = null;
    public ?int $eedrAddress = null;
    public ?int $spmcrAddress = null;
    public ?int $osccalAddress = null;
}
