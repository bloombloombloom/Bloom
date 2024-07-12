<?php
namespace Targets;

enum TargetPhysicalInterface: string
{
    case ISP = 'isp';
    case JTAG = 'jtag';
    case PDI = 'pdi';
    case UPDI = 'updi';
    case DEBUG_WIRE = 'debug_wire';

    public function supportsDebugging(): bool
    {
        return $this !== self::ISP;
    }

    public function marketingName(): string
    {
        return match ($this) {
            self::ISP => 'ISP',
            self::JTAG => 'JTAG',
            self::PDI => 'PDI',
            self::UPDI => 'UPDI',
            self::DEBUG_WIRE => 'debugWIRE',
        };
    }

    public function configValue(): ?string
    {
        return match ($this) {
            self::JTAG => 'jtag',
            self::PDI => 'pdi',
            self::UPDI => 'updi',
            self::DEBUG_WIRE => 'debug-wire',
            default => null
        };
    }
}