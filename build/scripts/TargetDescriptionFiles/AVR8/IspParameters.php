<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

class IspParameters
{
    public ?int $programModeTimeout = null;
    public ?int $programModeStabilizationDelay = null;
    public ?int $programModeCommandExecutionDelay = null;
    public ?int $programModeSyncLoops = null;
    public ?int $programModeByteDelay = null;
    public ?int $programModePollValue = null;
    public ?int $programModePollIndex = null;
    public ?int $programModePreDelay = null;
    public ?int $programModePostDelay = null;
    public ?int $readSignaturePollIndex = null;
    public ?int $readFusePollIndex = null;
    public ?int $readLockPollIndex = null;
}
