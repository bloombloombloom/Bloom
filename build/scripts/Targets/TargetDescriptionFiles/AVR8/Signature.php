<?php
namespace Targets\TargetDescriptionFiles\Avr8;

class Signature
{
    public ?int $byteZero = null;
    public ?int $byteOne = null;
    public ?int $byteTwo = null;

    public function toHex(): string
    {
        if ($this->byteZero === null || $this->byteOne === null || $this->byteTwo === null) {
            throw new \Exception("Cannot generate hex string of incomplete AVR8 target signature.");
        }

        return '0x' . substr('0' . dechex($this->byteZero), -2)
            . substr('0' . dechex($this->byteOne), -2)
            . substr('0' . dechex($this->byteTwo), -2)
        ;
    }
}
