<?php
namespace Targets\TargetDescriptionFiles\Avr8;

class PdiParameters
{
    public ?int $appSectionOffset = null;
    public ?int $bootSectionOffset = null;
    public ?int $eepromOffset = null;
    public ?int $fuseRegistersOffset = null;
    public ?int $lockRegistersOffset = null;
    public ?int $userSignaturesOffset = null;
    public ?int $productionSignaturesOffset = null;
    public ?int $ramOffset = null;
    public ?int $appSectionSize = null;
    public ?int $bootSectionSize = null;
    public ?int $flashPageSize = null;
    public ?int $eepromSize = null;
    public ?int $eepromPageSize = null;
    public ?int $nvmModuleBaseAddress = null;
    public ?int $signatureOffset = null;
}
