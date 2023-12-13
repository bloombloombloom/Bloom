<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

enum AvrPhysicalInterface: string
{
    case ISP = 'ISP';
    case JTAG = 'JTAG';
    case PDI = 'PDI';
    case UPDI = 'UPDI';
    case DEBUG_WIRE = 'DEBUG_WIRE';
}
