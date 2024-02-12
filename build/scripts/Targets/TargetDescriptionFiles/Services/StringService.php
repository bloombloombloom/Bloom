<?php
namespace Targets\TargetDescriptionFiles\Services;

class StringService
{
    public function tryStringToInt(?string $value): ?int
    {
        if ($value === null || strlen($value) === 0) {
            return null;
        }

        if (stripos($value, '0x') === 0) {
            return ctype_xdigit(substr($value, 2)) ? (int) hexdec($value) : null;
        }

        return ctype_digit($value) ? (int) $value : null;
    }

    public function tryIntToHex(?int $value, int $pad = 0): string
    {
        if ($value === null) {
            return '';
        }

        return '0x' . str_pad(strtoupper(dechex($value)), $pad, '0', STR_PAD_LEFT);
    }
}
