<?php
namespace Targets\TargetDescriptionFiles;

enum TargetFamily: string
{
    case AVR_8 = 'AVR8';
    case RISC_V = 'RISCV';
}