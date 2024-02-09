<?php
namespace Targets\TargetDescriptionFiles;

enum PinoutType: string
{
    case SOIC = 'soic';
    case SSOP = 'ssop';
    case DIP = 'dip';
    case QFN = 'qfn';
    case MLF = 'mlf';
    case DUAL_ROW_QFN = 'drqfn';
    case QFP = 'qfp';
    case BGA = 'bga';
}
