<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles;

enum PinoutType: string
{
    case SOIC = 'SOIC';
    case SSOP = 'SSOP';
    case DIP = 'DIP';
    case QFN = 'QFN';
    case QFP = 'QFP';
    case BGA = 'BGA';
}
