<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

class PdiParameters
{
    public ?int $appSectionStartAddress = null;
    public ?int $appSectionSize = null;
    public ?int $bootSectionStartAddress = null;
    public ?int $bootSectionSize = null;
    public ?int $appSectionPdiOffset = null;
    public ?int $bootSectionPdiOffset = null;
    public ?int $eepromPdiOffset = null;
    public ?int $ramPdiOffset = null;
    public ?int $fuseRegistersPdiOffset = null;
    public ?int $lockRegistersPdiOffset = null;
    public ?int $userSignaturesPdiOffset = null;
    public ?int $productSignaturesPdiOffset = null;
    public ?int $nvmModuleBaseAddress = null;
    public ?int $mcuModuleBaseAddress = null;
}
