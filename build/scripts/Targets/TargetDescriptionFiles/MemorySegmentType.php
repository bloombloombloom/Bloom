<?php
namespace Targets\TargetDescriptionFiles;

enum MemorySegmentType: string
{
    case ALIASED = 'aliased';
    case GENERAL_PURPOSE_REGISTERS = 'gp_registers';
    case REGISTERS = 'registers';
    case EEPROM = 'eeprom';
    case FLASH = 'flash';
    case FUSES = 'fuses';
    case IO = 'io';
    case RAM = 'ram';
    case LOCKBITS = 'lockbits';
    case OSCCAL = 'osccal';
    case PRODUCTION_SIGNATURES = 'production_signatures';
    case SIGNATURES = 'signatures';
    case USER_SIGNATURES = 'user_signatures';
}
