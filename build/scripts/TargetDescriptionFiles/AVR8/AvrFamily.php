<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

enum AvrFamily: string
{
    case MEGA = 'MEGA';
    case TINY = 'TINY';
    case XMEGA = 'XMEGA';
    case DB = 'DB';
    case DA = 'DA';
    case DD = 'DD';
    case EA = 'EA';
}
