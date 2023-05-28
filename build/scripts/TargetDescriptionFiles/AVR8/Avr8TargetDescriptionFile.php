<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

use Bloom\BuildScripts\TargetDescriptionFiles\AddressSpace;
use Bloom\BuildScripts\TargetDescriptionFiles\MemorySegment;
use Bloom\BuildScripts\TargetDescriptionFiles\Register;
use Bloom\BuildScripts\TargetDescriptionFiles\TargetDescriptionFile;

require_once __DIR__ . "/../TargetDescriptionFile.php";
require_once __DIR__ . "/Signature.php";
require_once __DIR__ . "/TargetParameters.php";
require_once __DIR__ . "/DebugWireParameters.php";
require_once __DIR__ . "/IspParameters.php";
require_once __DIR__ . "/JtagParameters.php";
require_once __DIR__ . "/PdiParameters.php";
require_once __DIR__ . "/UpdiParameters.php";
require_once __DIR__ . "/FuseBitsDescriptor.php";

class Avr8TargetDescriptionFile extends TargetDescriptionFile
{
    const AVR8_FAMILY_MEGA = 'MEGA';
    const AVR8_FAMILY_TINY = 'TINY';
    const AVR8_FAMILY_XMEGA = 'XMEGA';
    const AVR8_FAMILY_DB = 'DB';
    const AVR8_FAMILY_DA = 'DA';
    const AVR8_FAMILY_DD = 'DD';
    const AVR8_FAMILY_EA = 'EA';
    const AVR8_FAMILY_OTHER = 'OTHER';

    const AVR8_PHYSICAL_INTERFACE_ISP = 'ISP';
    const AVR8_PHYSICAL_INTERFACE_JTAG = 'JTAG';
    const AVR8_PHYSICAL_INTERFACE_PDI = 'PDI';
    const AVR8_PHYSICAL_INTERFACE_UPDI = 'UPDI';
    const AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE = 'DEBUG_WIRE';

    protected function init()
    {
        parent::init();
    }

    public function getSignature(): ?Signature
    {
        $byteZero = $this->getPropertyValue('signatures', 'signature0');
        $byteOne = $this->getPropertyValue('signatures', 'signature1');
        $byteTwo = $this->getPropertyValue('signatures', 'signature2');

        if (!empty($byteZero) && !empty($byteOne) && !empty($byteTwo)) {
            $signature = new Signature();
            $signature->byteZero = $this->stringToInt($byteZero);
            $signature->byteOne = $this->stringToInt($byteOne);
            $signature->byteTwo = $this->stringToInt($byteTwo);

            return $signature;
        }

        return null;
    }

    public function getFamily(): ?string
    {
        if (!empty($this->deviceAttributesByName['family'])) {
            if (stristr($this->deviceAttributesByName['family'], 'xmega') !== false) {
                return self::AVR8_FAMILY_XMEGA;

            } else if (stristr($this->deviceAttributesByName['family'], 'tiny') !== false) {
                return self::AVR8_FAMILY_TINY;

            } else if (stristr($this->deviceAttributesByName['family'], 'mega') !== false) {
                return self::AVR8_FAMILY_MEGA;

            } else if (strtolower(trim($this->deviceAttributesByName['family'])) == 'avr da') {
                return self::AVR8_FAMILY_DA;

            } else if (strtolower(trim($this->deviceAttributesByName['family'])) == 'avr db') {
                return self::AVR8_FAMILY_DB;

            } else if (strtolower(trim($this->deviceAttributesByName['family'])) == 'avr dd') {
                return self::AVR8_FAMILY_DD;

            } else if (strtolower(trim($this->deviceAttributesByName['family'])) == 'avr ea') {
                return self::AVR8_FAMILY_EA;
            }
        }

        return self::AVR8_FAMILY_OTHER;
    }

    public function getSupportedPhysicalInterfaces(): array
    {
        $physicalInterfacesByName = [
            'isp' => self::AVR8_PHYSICAL_INTERFACE_ISP,
            'debugwire' => self::AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE,
            'updi' => self::AVR8_PHYSICAL_INTERFACE_UPDI,
            'pdi' => self::AVR8_PHYSICAL_INTERFACE_PDI,
            'jtag' => self::AVR8_PHYSICAL_INTERFACE_JTAG,
        ];

        return array_filter(
            $physicalInterfacesByName,
            function(string $name) {
                return isset($this->physicalInterfacesByName[$name]);
            },
            ARRAY_FILTER_USE_KEY
        );
    }

    public function getSupportedDebugPhysicalInterfaces(): array
    {
        $physicalInterfacesByName = [
            'debugwire' => self::AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE,
            'updi' => self::AVR8_PHYSICAL_INTERFACE_UPDI,
            'pdi' => self::AVR8_PHYSICAL_INTERFACE_PDI,
            'jtag' => self::AVR8_PHYSICAL_INTERFACE_JTAG,
        ];

        return array_filter(
            $physicalInterfacesByName,
            function(string $name) {
                return isset($this->physicalInterfacesByName[$name]);
            },
            ARRAY_FILTER_USE_KEY
        );
    }

    public function getTargetParameters(): TargetParameters
    {
        $output = new TargetParameters();

        $registerMemorySegment = $this->getMemorySegment('data', 'regs');
        if ($registerMemorySegment instanceof MemorySegment) {
            $output->gpRegisterSize = $registerMemorySegment->size;
            $output->gpRegisterStartAddress = $registerMemorySegment->startAddress;
        }

        $programMemoryAddressSpace = $this->addressSpacesById['prog'] ?? null;
        if ($programMemoryAddressSpace instanceof AddressSpace) {
            $output->flashSize = $programMemoryAddressSpace->size;
            $output->flashStartAddress = $programMemoryAddressSpace->startAddress;
        }

        $programMemorySegment = $this->getMemorySegment('prog', 'flash');
        if ($programMemorySegment instanceof MemorySegment) {
            $output->flashPageSize = $programMemorySegment->pageSize;
        }

        $ramMemorySegment = $this->getMemorySegment('data', 'ram');
        if ($ramMemorySegment instanceof MemorySegment) {
            $output->ramStartAddress = $ramMemorySegment->startAddress;
            $output->ramSize = $ramMemorySegment->size;
        }

        $eepromMemorySegment = $this->getMemorySegment(
            'data',
            'eeprom',
            'eeprom'
        );
        if ($eepromMemorySegment instanceof MemorySegment) {
            $output->eepromStartAddress = $eepromMemorySegment->startAddress;
            $output->eepromSize = $eepromMemorySegment->size;
            $output->eepromPageSize = $eepromMemorySegment->pageSize;

        } else if (isset($this->addressSpacesById['eeprom'])) {
            $eepromAddressSpace = $this->addressSpacesById['eeprom'];
            $output->eepromStartAddress = $eepromAddressSpace->startAddress;

            $eepromMemorySegment = $this->getMemorySegment('eeprom', 'eeprom');
            if ($eepromMemorySegment instanceof MemorySegment) {
                $output->eepromSize = $eepromMemorySegment->size;
                $output->eepromPageSize = $eepromMemorySegment->pageSize;
            }
        }

        $eepromAddressRegister = $this->getModuleRegister(
            'eeprom',
            'eeprom',
            'eear'
        );
        if ($eepromAddressRegister instanceof Register) {
            $output->eepromAddressRegisterLow = $eepromAddressRegister->offset;
            $output->eepromAddressRegisterHigh = ($eepromAddressRegister->size == 2)
                ? $eepromAddressRegister->offset + 1
                : $eepromAddressRegister->offset;

        } else {
            $eepromAddressRegisterLow = $this->getModuleRegister(
                'eeprom',
                'eeprom',
                'eearl'
            );
            if ($eepromAddressRegisterLow instanceof Register) {
                $output->eepromAddressRegisterLow = $eepromAddressRegisterLow->offset;
                $output->eepromAddressRegisterHigh = $eepromAddressRegisterLow->offset;
            }

            $eepromAddressRegisterHigh = $this->getModuleRegister(
                'eeprom',
                'eeprom',
                'eearh'
            );
            if ($eepromAddressRegisterHigh instanceof Register) {
                $output->eepromAddressRegisterHigh = $eepromAddressRegisterHigh->offset;
            }
        }

        $eepromDataRegister = $this->getModuleRegister(
            'eeprom',
            'eeprom',
            'eedr'
        );
        if ($eepromDataRegister instanceof Register) {
            $output->eepromDataRegisterAddress = $eepromDataRegister->offset;
        }

        $eepromControlRegister = $this->getModuleRegister(
            'eeprom',
            'eeprom',
            'eecr'
        );
        if ($eepromControlRegister instanceof Register) {
            $output->eepromControlRegisterAddress = $eepromControlRegister->offset;
        }

        $statusRegister = $this->getModuleRegister(
            'cpu',
            'cpu',
            'sreg'
        );
        if ($statusRegister instanceof Register) {
            $output->statusRegisterStartAddress = $statusRegister->offset;
            $output->statusRegisterSize = $statusRegister->size;
        }

        $stackPointerRegister = $this->getModuleRegister(
            'cpu',
            'cpu',
            'sp'
        );
        if ($stackPointerRegister instanceof Register) {
            $output->stackPointerRegisterLowAddress = $stackPointerRegister->offset;
            $output->stackPointerRegisterSize = $stackPointerRegister->size;

        } else {
            $stackPointerRegisterLow = $this->getModuleRegister(
                'cpu',
                'cpu',
                'spl'
            );
            if ($stackPointerRegisterLow instanceof Register) {
                $output->stackPointerRegisterLowAddress = $stackPointerRegisterLow->offset;
                $output->stackPointerRegisterSize = $stackPointerRegisterLow->size;
            }

            $stackPointerRegisterHigh = $this->getModuleRegister(
                'cpu',
                'cpu',
                'sph'
            );
            if ($stackPointerRegisterHigh instanceof Register) {
                $output->stackPointerRegisterSize += $stackPointerRegisterHigh->size;
            }
        }

        return $output;
    }

    public function getDebugWireParameters(): DebugWireParameters
    {
        $output = new DebugWireParameters();

        $output->ocdRevision = $this->stringToInt(
            $this->getPropertyValue('ocd', 'ocd_revision')
        );
        $output->ocdDataRegister = $this->stringToInt(
            $this->getPropertyValue('ocd', 'ocd_datareg')
        );

        $spmcsRegister = $this->getModuleRegister(
                'cpu',
                'cpu',
                'spmcsr'
            ) ?? $this->getModuleRegister(
                'cpu',
                'cpu',
                'spmcr'
            ) ?? $this->getModuleRegister(
                'boot_load',
                'boot_load',
                'spmcr'
            ) ?? $this->getModuleRegister(
                'boot_load',
                'boot_load',
                'spmcsr'
            );
        if ($spmcsRegister instanceof Register) {
            $output->spmcRegisterStartAddress = $spmcsRegister->offset;
        }

        $osccalRegister = $this->getModuleRegister(
                'cpu',
                'cpu',
                'osccal'
            ) ?? $this->getModuleRegister(
                'cpu',
                'cpu',
                'osccal0'
            ) ?? $this->getModuleRegister(
                'cpu',
                'cpu',
                'osccal1'
            ) ?? $this->getModuleRegister(
                'cpu',
                'cpu',
                'fosccal'
            ) ?? $this->getModuleRegister(
                'cpu',
                'cpu',
                'sosccala'
            );
        if ($osccalRegister instanceof Register) {
            $output->osccalAddress = $osccalRegister->offset;
        }

        return $output;
    }

    public function getIspParameters(): IspParameters
    {
        $output = new IspParameters();

        $output->programModeTimeout = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_timeout')
        );
        $output->programModeStabilizationDelay = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_stabdelay')
        );
        $output->programModeCommandExecutionDelay = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_cmdexedelay')
        );
        $output->programModeSyncLoops = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_synchloops')
        );
        $output->programModeByteDelay = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_bytedelay')
        );
        $output->programModePollValue = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_pollvalue')
        );
        $output->programModePollIndex = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_pollindex')
        );
        $output->programModePreDelay = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispleaveprogmode_predelay')
        );
        $output->programModePostDelay = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispleaveprogmode_postdelay')
        );
        $output->readSignaturePollIndex = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispreadsign_pollindex')
        );
        $output->readFusePollIndex = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispreadfuse_pollindex')
        );
        $output->readLockPollIndex = $this->stringToInt(
            $this->getPropertyValue('isp_interface', 'ispreadlock_pollindex')
        );

        return $output;
    }

    public function getJtagParameters(): JtagParameters
    {
        $output = new JtagParameters();

        $output->ocdRevision = $this->stringToInt(
            $this->getPropertyValue('ocd', 'ocd_revision')
        );
        $output->ocdDataRegister = $this->stringToInt(
            $this->getPropertyValue('ocd', 'ocd_datareg')
        );

        $spmcsRegister = $this->getModuleRegister(
                'cpu',
                'cpu',
                'spmcsr'
            ) ?? $this->getModuleRegister(
                'cpu',
                'cpu',
                'spmcr'
            ) ?? $this->getModuleRegister(
                'boot_load',
                'boot_load',
                'spmcr'
            ) ?? $this->getModuleRegister(
                'boot_load',
                'boot_load',
                'spmcsr'
            );
        if ($spmcsRegister instanceof Register) {
            $output->spmcRegisterStartAddress = $spmcsRegister->offset;
        }

        $osccalRegister = $this->getModuleRegister(
                'cpu',
                'cpu',
                'osccal'
            ) ?? $this->getModuleRegister(
                'cpu',
                'cpu',
                'osccal0'
            ) ?? $this->getModuleRegister(
                'cpu',
                'cpu',
                'osccal1'
            ) ?? $this->getModuleRegister(
                'cpu',
                'cpu',
                'fosccal'
            ) ?? $this->getModuleRegister(
                'cpu',
                'cpu',
                'sosccala'
            );
        if ($osccalRegister instanceof Register) {
            $output->osccalAddress = $osccalRegister->offset;
        }

        return $output;
    }

    public function getPdiParameters(): PdiParameters
    {
        $output = new PdiParameters();

        $applicationSectionMemorySegment = $this->getMemorySegment(
            'prog',
            'flash',
            'app_section'
        );
        if ($applicationSectionMemorySegment instanceof MemorySegment) {
            $output->appSectionStartAddress = $applicationSectionMemorySegment->startAddress;
            $output->appSectionSize = $applicationSectionMemorySegment->size;
        }

        $bootSectionMemorySegment = $this->getMemorySegment(
            'prog',
            'flash',
            'boot_section_1'
        ) ?? $this->getMemorySegment(
            'prog',
            'flash',
            'boot_section'
        );

        if ($bootSectionMemorySegment instanceof MemorySegment) {
            $output->bootSectionStartAddress = $bootSectionMemorySegment->startAddress;
            $output->bootSectionSize = $bootSectionMemorySegment->size;
        }

        $output->appSectionPdiOffset = $this->stringToInt(
            $this->getPropertyValue('pdi_interface', 'app_section_offset')
        );
        $output->bootSectionPdiOffset = $this->stringToInt(
            $this->getPropertyValue('pdi_interface', 'boot_section_offset')
        );
        $output->ramPdiOffset = $this->stringToInt(
            $this->getPropertyValue('pdi_interface', 'datamem_offset')
        );
        $output->eepromPdiOffset = $this->stringToInt(
            $this->getPropertyValue('pdi_interface', 'eeprom_offset')
        );
        $output->userSignaturesPdiOffset = $this->stringToInt(
            $this->getPropertyValue('pdi_interface', 'user_signatures_offset')
        );
        $output->productSignaturesPdiOffset = $this->stringToInt(
            $this->getPropertyValue('pdi_interface', 'prod_signatures_offset')
        );
        $output->fuseRegistersPdiOffset = $this->stringToInt(
            $this->getPropertyValue('pdi_interface', 'fuse_registers_offset')
        );
        $output->lockRegistersPdiOffset = $this->stringToInt(
            $this->getPropertyValue('pdi_interface', 'lock_registers_offset')
        );

        $output->nvmModuleBaseAddress = $this->getPeripheralModuleRegisterGroupOffset(
            'nvm',
            'nvm',
            'nvm'
        );
        $output->mcuModuleBaseAddress = $this->getPeripheralModuleRegisterGroupOffset(
            'mcu',
            'mcu',
            'mcu'
        );

        return $output;
    }

    public function getUpdiParameters(): UpdiParameters
    {
        $output = new UpdiParameters();
        $output->nvmModuleBaseAddress = $this->getPeripheralModuleRegisterGroupOffset(
            'nvmctrl',
            'nvmctrl',
            'nvmctrl'
        );

        $output->ocdBaseAddress = $this->stringToInt(
            $this->getPropertyValue('updi_interface', 'ocd_base_addr')
        );
        $output->programMemoryStartAddress = $this->stringToInt(
            $this->getPropertyValue('updi_interface', 'progmem_offset')
        );

        $signatureMemorySegment = $this->getMemorySegment(
            'data',
            'signatures',
            'signatures'
        );
        if ($signatureMemorySegment instanceof MemorySegment) {
            $output->signatureSegmentSize = $signatureMemorySegment->size;
            $output->signatureSegmentStartAddress = $signatureMemorySegment->startAddress;
        }

        $fuseMemorySegment = $this->getMemorySegment(
            'data',
            'fuses',
            'fuses'
        );
        if ($fuseMemorySegment instanceof MemorySegment) {
            $output->fuseSegmentSize = $fuseMemorySegment->size;
            $output->fuseSegmentStartAddress = $fuseMemorySegment->startAddress;
        }

        $lockbitsMemorySegment = $this->getMemorySegment(
            'data',
            'lockbits',
            'lockbits'
        );
        if ($lockbitsMemorySegment instanceof MemorySegment) {
            $output->lockbitsSegmentStartAddress = $lockbitsMemorySegment->startAddress;
        }

        return $output;
    }

    public function getFuseBitsDescriptor(string $fuseBitName): ?FuseBitsDescriptor
    {
        $fuseModule = $this->modulesByName['fuse'] ?? null;
        if (!empty($fuseModule)) {
            $fuseRegisterGroup = $fuseModule->registerGroupsMappedByName['fuse'] ?? null;

            if (empty($fuseRegisterGroup)) {
                $fuseRegisterGroup = $fuseModule->registerGroupsMappedByName['nvm_fuses'] ?? null;
            }

            if (!empty($fuseRegisterGroup)) {
                foreach ($fuseRegisterGroup->registersMappedByName as $fuseType => $fuseRegister) {
                    if (isset($fuseRegister->bitFieldsByName[$fuseBitName])) {
                        return new FuseBitsDescriptor($fuseType);
                    }
                }
            }
        }

        // Try the NVM module
        $nvmModule = $this->modulesByName['nvm'] ?? null;
        if (!empty($nvmModule)) {
            $fuseRegisterGroup = $nvmModule->registerGroupsMappedByName['nvm_fuses'] ?? null;

            if (!empty($fuseRegisterGroup)) {
                foreach ($fuseRegisterGroup->registersMappedByName as $fuseType => $fuseRegister) {
                    if (isset($fuseRegister->bitFieldsByName[$fuseBitName])) {
                        return new FuseBitsDescriptor($fuseType);
                    }
                }
            }
        }

        return null;
    }

    public function validate(): array
    {
        $failures = parent::validate();

        if (is_null($this->getSignature())) {
            $failures[] = "Missing or incomplete AVR signature.";
        }

        $physicalInterfaces = $this->getSupportedPhysicalInterfaces();
        $debugPhysicalInterfaces = $this->getSupportedDebugPhysicalInterfaces();

        if (empty($debugPhysicalInterfaces)) {
            $failures[] = 'Target does not support any known AVR8 debug interface - the TDF will need to be deleted.'
                . ' Aborting validation.';
            return $failures;
        }

        $family = $this->getFamily();
        if (is_null($family) || $family == self::AVR8_FAMILY_OTHER) {
            $failures[] = 'Unknown AVR8 family';
        }

        $targetParameters = $this->getTargetParameters();

        if (is_null($targetParameters->stackPointerRegisterSize)) {
            $failures[] = 'Missing stack pointer register size.';
        }

        if ($targetParameters->stackPointerRegisterSize > 2) {
            // The AVR architecture implementation in GDB expects all SP registers to be a maximum of two bytes in size.
            $failures[] = 'Stack pointer register size (' . $targetParameters->stackPointerRegisterSize . ') exceeds maximum (2).';
        }

        if ($targetParameters->stackPointerRegisterSize < 1) {
            $failures[] = 'Stack pointer register size (' . $targetParameters->stackPointerRegisterSize . ') is less than 1.';
        }

        if (is_null($targetParameters->stackPointerRegisterLowAddress)) {
            $failures[] = 'Missing stack pointer register start address.';
        }

        if (is_null($targetParameters->statusRegisterStartAddress)) {
            $failures[] = 'Missing status register start address.';
        }

        if (is_null($targetParameters->statusRegisterSize)) {
            $failures[] = 'Missing status register size.';

        } else if ($targetParameters->statusRegisterSize > 1) {
            $failures[] = 'Status register size exceeds 1';
        }

        if (is_null($targetParameters->flashSize)) {
            $failures[] = 'Missing flash size.';
        }

        if (is_null($targetParameters->flashPageSize)) {
            $failures[] = 'Missing flash page size.';
        }

        if (is_null($targetParameters->flashStartAddress)) {
            $failures[] = 'Missing flash start address.';
        }

        if (is_null($targetParameters->ramStartAddress)) {
            $failures[] = 'Missing ram start address.';
        }

        if (is_null($targetParameters->eepromSize)) {
            $failures[] = 'Missing eeprom size.';
        }

        if ($targetParameters->eepromSize < 64) {
            /*
             * All AVR8 targets supported by Bloom have at least 64 bytes of EEPROM. This is assumed in some areas of
             * Bloom's code.
             *
             * The purpose of this check is to remind me to address those areas of Bloom's code before adding support
             * for an AVR8 target with no EEPROM.
             */
            $failures[] = 'Unexpected eeprom size.';
        }

        if (is_null($targetParameters->eepromPageSize)) {
            $failures[] = 'Missing eeprom page size.';
        }

        if (is_null($targetParameters->eepromStartAddress)) {
            $failures[] = 'Missing eeprom start address.';
        }

        if (in_array(self::AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE, $debugPhysicalInterfaces)) {
            $debugWireParameters = $this->getDebugWireParameters();
            $ispParameters = $this->getIspParameters();

            if (is_null($debugWireParameters->ocdRevision)) {
                $failures[] = 'Missing OCD revision.';
            }

            if (is_null($debugWireParameters->ocdDataRegister)) {
                $failures[] = 'Missing OCD data register address.';
            }

            if (is_null($debugWireParameters->spmcRegisterStartAddress)) {
                $failures[] = 'Missing store program memory control register start address.';
            }

            if (is_null($debugWireParameters->osccalAddress)) {
                $failures[] = 'Missing oscillator calibration register address.';
            }

            if (!in_array(self::AVR8_PHYSICAL_INTERFACE_ISP, $physicalInterfaces)) {
                $failures[] = 'Missing ISP interface for debugWire target.';
            }

            if (is_null($ispParameters->programModeTimeout)) {
                $failures[] = 'Missing ispenterprogmode_timeout ISP parameter.';
            }

            if (is_null($ispParameters->programModeStabilizationDelay)) {
                $failures[] = 'Missing ispenterprogmode_stabdelay ISP parameter.';
            }

            if (is_null($ispParameters->programModeCommandExecutionDelay)) {
                $failures[] = 'Missing ispenterprogmode_cmdexedelay ISP parameter.';
            }

            if (is_null($ispParameters->programModeSyncLoops)) {
                $failures[] = 'Missing ispenterprogmode_synchloops ISP parameter.';
            }

            if (is_null($ispParameters->programModeByteDelay)) {
                $failures[] = 'Missing ispenterprogmode_bytedelay ISP parameter.';
            }

            if (is_null($ispParameters->programModePollValue)) {
                $failures[] = 'Missing ispenterprogmode_pollvalue ISP parameter.';
            }

            if (is_null($ispParameters->programModePollIndex)) {
                $failures[] = 'Missing ispenterprogmode_pollindex ISP parameter.';
            }

            if (is_null($ispParameters->programModePreDelay)) {
                $failures[] = 'Missing ispleaveprogmode_predelay ISP parameter.';
            }

            if (is_null($ispParameters->programModePostDelay)) {
                $failures[] = 'Missing ispleaveprogmode_postdelay ISP parameter.';
            }

            if (is_null($ispParameters->readSignaturePollIndex)) {
                $failures[] = 'Missing ispreadsign_pollindex ISP parameter.';
            }

            if (is_null($ispParameters->readFusePollIndex)) {
                $failures[] = 'Missing ispreadfuse_pollindex ISP parameter.';
            }

            if (is_null($ispParameters->readLockPollIndex)) {
                $failures[] = 'Missing ispreadlock_pollindex ISP parameter.';
            }

            $dwenFuseBitDescriptor = $this->getFuseBitsDescriptor('dwen');
            if (empty($dwenFuseBitDescriptor)) {
                $failures[] = 'Could not find DWEN fuse bit field for debugWire target.';

            } else {
                static $validFuseTypes = [
                    FuseBitsDescriptor::FUSE_TYPE_LOW,
                    FuseBitsDescriptor::FUSE_TYPE_HIGH,
                    FuseBitsDescriptor::FUSE_TYPE_EXTENDED,
                ];

                if (!in_array($dwenFuseBitDescriptor->fuseType, $validFuseTypes)) {
                    $failures[] = 'Invalid/unknown fuse byte type for DWEN fuse bit.';
                }
            }
        }

        if (
            in_array(self::AVR8_PHYSICAL_INTERFACE_JTAG, $debugPhysicalInterfaces)
            && $family == self::AVR8_FAMILY_MEGA
        ) {
            $jtagParameters = $this->getJtagParameters();

            if (is_null($jtagParameters->ocdRevision)) {
                $failures[] = 'Missing OCD revision.';
            }

            if (is_null($jtagParameters->ocdDataRegister)) {
                $failures[] = 'Missing OCD data register address.';
            }

            if (is_null($jtagParameters->spmcRegisterStartAddress)) {
                $failures[] = 'Missing store program memory control register start address.';
            }

            if (is_null($jtagParameters->osccalAddress)) {
                $failures[] = 'Missing oscillator calibration register address.';
            }

            if (empty($this->getFuseBitsDescriptor('ocden'))) {
                $failures[] = 'Could not find OCDEN fuse bit field for JTAG target.';
            }

            if (empty($this->getFuseBitsDescriptor('jtagen'))) {
                $failures[] = 'Could not find JTAGEN fuse bit field for JTAG target.';
            }
        }

        if (in_array(self::AVR8_PHYSICAL_INTERFACE_PDI, $debugPhysicalInterfaces)) {
            $pdiParameters = $this->getPdiParameters();

            if (is_null($pdiParameters->appSectionPdiOffset)) {
                $failures[] = 'Missing app section PDI offset.';
            }

            if (is_null($pdiParameters->bootSectionPdiOffset)) {
                $failures[] = 'Missing boot section PDI offset.';
            }

            if (is_null($pdiParameters->ramPdiOffset)) {
                $failures[] = 'Missing datamem PDI offset.';
            }

            if (is_null($pdiParameters->eepromPdiOffset)) {
                $failures[] = 'Missing eeprom PDI offset.';
            }

            if (is_null($pdiParameters->userSignaturesPdiOffset)) {
                $failures[] = 'Missing user signatures PDI offset.';
            }

            if (is_null($pdiParameters->productSignaturesPdiOffset)) {
                $failures[] = 'Missing product signatures PDI offset.';
            }

            if (is_null($pdiParameters->lockRegistersPdiOffset)) {
                $failures[] = 'Missing lock registers PDI offset.';
            }

            if (is_null($pdiParameters->nvmModuleBaseAddress)) {
                $failures[] = 'Missing NVM module base address.';
            }

            if (is_null($pdiParameters->mcuModuleBaseAddress)) {
                $failures[] = 'Missing MCU module base address.';
            }

            if (is_null($pdiParameters->appSectionStartAddress)) {
                $failures[] = 'Missing APP section start address';
            }

            if (is_null($pdiParameters->appSectionSize)) {
                $failures[] = 'Missing APP section size';
            }
        }

        if (in_array(Avr8TargetDescriptionFile::AVR8_PHYSICAL_INTERFACE_UPDI, $debugPhysicalInterfaces)) {
            $updiParameters = $this->getUpdiParameters();
            if (is_null($updiParameters->nvmModuleBaseAddress)) {
                $failures[] = 'Missing NVM base address.';
            }

            if (is_null($updiParameters->programMemoryStartAddress)) {
                $failures[] = 'Missing UPDI program memory offset.';

            } else if ($updiParameters->programMemoryStartAddress > 0xFFFFFF) {
                /*
                 * Due to size constraints of EDBG AVR8 parameters for UPDI sessions, the program memory offset must
                 * fit into a 24-bit integer.
                 */
                $failures[] = 'UPDI program memory offset exceeds maximum value for 24-bit unsigned integer.';
            }

            if (!is_null($targetParameters->flashPageSize) && $targetParameters->flashPageSize > 0xFFFF) {
                $failures[] = 'Flash page size exceeds maximum value for 16-bit unsigned integer.';
            }

            if (is_null($updiParameters->ocdBaseAddress)) {
                $failures[] = 'Missing OCD base address.';

            } else if ($updiParameters->ocdBaseAddress > 0xFFFF) {
                $failures[] = 'UPDI OCD base address exceeds maximum value for 16-bit unsigned integer.';
            }

            if (is_null($updiParameters->signatureSegmentStartAddress)) {
                $failures[] = 'Missing signature segment start address.';
            }

            if (is_null($updiParameters->fuseSegmentSize)) {
                $failures[] = 'Missing fuse segment size.';
            }

            if (is_null($updiParameters->fuseSegmentStartAddress)) {
                $failures[] = 'Missing fuses segment start address.';
            }

            if (is_null($updiParameters->lockbitsSegmentStartAddress)) {
                $failures[] = 'Missing lockbits segment start address.';
            }
        }

        if (
            in_array(Avr8TargetDescriptionFile::AVR8_PHYSICAL_INTERFACE_JTAG, $debugPhysicalInterfaces)
            || in_array(Avr8TargetDescriptionFile::AVR8_PHYSICAL_INTERFACE_UPDI, $debugPhysicalInterfaces)
        ) {
            if (empty($this->getFuseBitsDescriptor('eesave'))) {
                $failures[] = 'Could not find EESAVE fuse bit field for JTAG/UPDI target.';
            }
        }

        $portPeripheralModule = $this->peripheralModulesByName['port'] ?? null;
        if (empty($portPeripheralModule)) {
            $failures[] = 'PORT peripheral module not found.';

        } else {
            $portModule = $this->modulesByName['port'];
            foreach ($portPeripheralModule->instancesMappedByName as $portName => $portInstance) {
                if (strlen($portName) === 5 && strpos($portName, "port") === 0) {
                    $portSuffix = substr($portName, strlen($portName) - 1, 1);
                    if (empty($portInstance->signals)) {
                        $failures[] = 'No signals defined for port ' . $portInstance->name . ' in PORT peripheral module.';
                    }

                    if (empty($portModule->registerGroupsMappedByName['port' . $portSuffix])) {

                        if (empty($portModule->registerGroupsMappedByName['port']->registersMappedByName['dir'])) {
                            $failures[] = 'Could not find PORT register group in PORT module, for port '
                                . $portName . ', using suffix ' . $portSuffix;
                        }
                    }
                }
            }
        }

        return $failures;
    }
}
