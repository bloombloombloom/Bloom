<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

class JtagParameters
{
    public ?int $ocdRevision = null;
    public ?int $ocdDataRegister = null;
    public ?int $spmcRegisterStartAddress = null;
    public ?int $osccalAddress = null;
}
