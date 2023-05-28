<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

class TargetParameters
{
    public ?int $gpRegisterStartAddress = null;
    public ?int $gpRegisterSize = null;
    public ?int $flashStartAddress = null;
    public ?int $flashSize = null;
    public ?int $flashPageSize = null;
    public ?int $ramStartAddress = null;
    public ?int $ramSize = null;
    public ?int $eepromSize = null;
    public ?int $eepromPageSize = null;
    public ?int $eepromStartAddress = null;
    public ?int $eepromAddressRegisterHigh = null;
    public ?int $eepromAddressRegisterLow = null;
    public ?int $eepromDataRegisterAddress = null;
    public ?int $eepromControlRegisterAddress = null;
    public ?int $statusRegisterStartAddress = null;
    public ?int $statusRegisterSize = null;
    public ?int $stackPointerRegisterLowAddress = null;
    public ?int $stackPointerRegisterSize = null;
}
