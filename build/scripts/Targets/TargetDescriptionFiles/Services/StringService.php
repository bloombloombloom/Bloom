<?php
namespace Targets\TargetDescriptionFiles\Services;

class StringService
{
    public function tryStringToInt(?string $value): ?int
    {
        if ($value === null || strlen($value) === 0) {
            return null;
        }

        return stristr($value, '0x') !== false ? (int) hexdec($value) : (int) $value;
    }

    public function tryIntToHex(?int $value, int $pad = 0): string
    {
        if ($value === null) {
            return '';
        }

        return '0x' . str_pad(strtoupper(dechex($value)), $pad, '0', STR_PAD_LEFT);
    }
}
