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
    public ?int $ocdDataRegister = null;
    public ?int $eepromAddressRegisterHigh = null;
    public ?int $eepromAddressRegisterLow = null;
    public ?int $eepromControlRegisterAddress = null;
    public ?int $eepromDataRegisterAddress = null;
    public ?int $spmcRegisterStartAddress = null;
    public ?int $osccalAddress = null;
}
