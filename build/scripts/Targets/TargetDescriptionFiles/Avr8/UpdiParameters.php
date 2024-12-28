<?php
namespace Targets\TargetDescriptionFiles\Avr8;

class UpdiParameters
{
    public ?int $programMemoryOffset = null;
    public ?int $flashPageSize = null;
    public ?int $eepromPageSize = null;
    public ?int $nvmModuleBaseAddress = null;
    public ?int $ocdBaseAddress = null;
    public ?int $flashSize = null;
    public ?int $eepromSize = null;
    public ?int $eepromStartAddress = null;
    public ?int $signatureSegmentStartAddress = null;
    public ?int $fuseSegmentStartAddress = null;
    public ?int $fuseSegmentSize = null;
    public ?int $lockbitsSegmentStartAddress = null;
}
