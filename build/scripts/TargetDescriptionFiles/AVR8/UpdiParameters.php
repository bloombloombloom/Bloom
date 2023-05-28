<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

class UpdiParameters
{
    public ?int $nvmModuleBaseAddress = null;
    public ?int $ocdBaseAddress = null;
    public ?int $programMemoryStartAddress = null;
    public ?int $signatureSegmentStartAddress = null;
    public ?int $signatureSegmentSize = null;
    public ?int $fuseSegmentStartAddress = null;
    public ?int $fuseSegmentSize = null;
    public ?int $lockbitsSegmentStartAddress = null;
}
