<?php
namespace Targets\TargetDescriptionFiles\AVR8\Services;

use Targets\TargetDescriptionFiles\Avr8\Avr8TargetDescriptionFile;
use Targets\TargetDescriptionFiles\Avr8\AvrFamily;
use Targets\TargetDescriptionFiles\Avr8\AvrPhysicalInterface;
use Targets\TargetDescriptionFiles\Avr8\DebugWireParameters;
use Targets\TargetDescriptionFiles\Avr8\IspParameters;
use Targets\TargetDescriptionFiles\Avr8\JtagParameters;
use Targets\TargetDescriptionFiles\Avr8\PdiParameters;
use Targets\TargetDescriptionFiles\Avr8\UpdiParameters;
use Targets\TargetRegister;
use Targets\TargetRegisterGroup;

require_once __DIR__ . '/../../Services/ValidationService.php';
require_once __DIR__ . '/../Avr8TargetDescriptionFile.php';

class ValidationService extends \Targets\TargetDescriptionFiles\Services\ValidationService
{
    public function validateAvr8Tdf(Avr8TargetDescriptionFile $tdf): array
    {
        $failures = $this->validateTdf($tdf);

        if ($tdf->getMemorySegment('prog', 'internal_program_memory') === null) {
            $failures[] = 'Missing "internal_program_memory" memory segment';
        }

        if ($tdf->getMemorySegment('data', 'internal_ram') === null) {
            $failures[] = 'Missing "internal_ram" memory segment';
        }

        if (
            $tdf->getMemorySegment('data', 'internal_eeprom') === null
            && $tdf->getMemorySegment('eeprom', 'internal_eeprom') === null
        ) {
            $failures[] = 'Missing "internal_eeprom" memory segment';
        }

        if (
            $tdf->getMemorySegment('data', 'io') === null
            && $tdf->getMemorySegment('data', 'mapped_io') === null
        ) {
            $failures[] = 'Missing IO memory segment';
        }

        if (
            $tdf->getMemorySegment('data', 'signatures') === null
            && $tdf->getMemorySegment('signatures', 'signatures') === null
        ) {
            $failures[] = 'Missing signatures memory segment';
        }

        if ($tdf->getSignature() === null) {
            $failures[] = "Missing or incomplete AVR signature.";
        }

        $physicalInterfaces = $tdf->getSupportedPhysicalInterfaces();
        $debugPhysicalInterfaces = $tdf->getSupportedDebugPhysicalInterfaces();

        if (empty($debugPhysicalInterfaces)) {
            $failures[] = 'Target does not support any known AVR8 debug interface - the TDF will need to be deleted.'
                . ' Aborting validation';
            return $failures;
        }

        $family = $tdf->getAvrFamily();
        if ($family === null) {
            $failures[] = 'Unknown AVR8 family';
        }

        if (in_array(AvrPhysicalInterface::DEBUG_WIRE, $debugPhysicalInterfaces)) {
            $failures = array_merge($failures, $this->validateDebugWireParameters($tdf->getDebugWireParameters()));
            $failures = array_merge($failures, $this->validateIspParameters($tdf->getIspParameters()));

            if (!in_array(AvrPhysicalInterface::ISP, $physicalInterfaces)) {
                $failures[] = 'Missing ISP interface for debugWIRE target';
            }

            $dwenFuseBitDescriptor = $tdf->getFuseBitsDescriptor('dwen');
            if (empty($dwenFuseBitDescriptor)) {
                $failures[] = 'Could not find DWEN fuse bit field for debugWIRE target';

            } else {
                static $validFuseTypes = [
                    'low',
                    'high',
                    'extended',
                ];

                if (!in_array($dwenFuseBitDescriptor->fuseType, $validFuseTypes)) {
                    $failures[] = 'Invalid/unknown fuse byte type for DWEN fuse bit';
                }
            }
        }

        if (
            in_array(AvrPhysicalInterface::JTAG, $debugPhysicalInterfaces)
            && $family == AvrFamily::MEGA
        ) {
            $failures = array_merge($failures, $this->validateJtagParameters($tdf->getJtagParameters()));

            if (empty($tdf->getFuseBitsDescriptor('ocden'))) {
                $failures[] = 'Could not find OCDEN fuse bit field for JTAG target';
            }

            if (empty($tdf->getFuseBitsDescriptor('jtagen'))) {
                $failures[] = 'Could not find JTAGEN fuse bit field for JTAG target';
            }
        }

        if (in_array(AvrPhysicalInterface::PDI, $debugPhysicalInterfaces)) {
            $failures = array_merge($failures, $this->validatePdiParameters($tdf->getPdiParameters()));
        }

        if (in_array(AvrPhysicalInterface::UPDI, $debugPhysicalInterfaces)) {
            $failures = array_merge($failures, $this->validateUpdiParameters($tdf->getUpdiParameters()));
        }

        if (
            in_array(AvrPhysicalInterface::JTAG, $debugPhysicalInterfaces)
            || in_array(AvrPhysicalInterface::UPDI, $debugPhysicalInterfaces)
        ) {
            if (empty($tdf->getFuseBitsDescriptor('eesave'))) {
                $failures[] = 'Could not find EESAVE fuse bit field for JTAG/UPDI target';
            }
        }

        // Ensure that all port peripherals have at least one signal and a port register group.
        foreach ($tdf->getPeripheralsOfModule('gpio_port') as $portPeripheral) {
            if (
                stripos($portPeripheral->name, 'vport') !== false
                || stripos($portPeripheral->name, 'port_cfg') !== false
            ) {
                // Ignore virtual port and port config peripherals
                continue;
            }

            if (empty($portPeripheral->signals)) {
                $failures[] = 'No signals defined for port peripheral "' . $portPeripheral->name . '"';
            }

            $alternativePortRegisterGroupKey = 'port' . substr(strtolower($portPeripheral->name), -1);
            $portRegisterGroup = $tdf->getTargetRegisterGroup($portPeripheral->key, 'port')
                ?? $tdf->getTargetRegisterGroup($portPeripheral->key, $alternativePortRegisterGroupKey);

            if (!$portRegisterGroup instanceof TargetRegisterGroup) {
                $failures[] = 'Missing port register group in port peripheral "' . $portPeripheral->name . '"';

            } else {
                $alternativePortRegisterKey = 'port' . substr(strtolower($portPeripheral->name), -1);
                $portRegister = $portRegisterGroup->getRegister('out')
                    ?? $portRegisterGroup->getRegister($alternativePortRegisterKey);

                if (!$portRegister instanceof TargetRegister) {
                    $failures[] = 'Missing port register in port peripheral "' . $portPeripheral->name . '"';
                }
            }
        }

        return $failures;
    }

    private function validateDebugWireParameters(DebugWireParameters $parameters): array
    {
        $failures = [];

        if ($parameters->flashPageSize === null) {
            $failures[] = 'Missing flash page size';

        } elseif ($parameters->flashPageSize > 0xFFFF) {
            $failures[] = 'Flash page size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->flashSize === null) {
            $failures[] = 'Missing flash size';

        } elseif ($parameters->flashSize > 0xFFFFFFFF) {
            $failures[] = 'Flash size exceeds 0xFFFFFFFF - corresponding EDBG device parameter size is 32 bits';
        }

        if ($parameters->flashStartAddress === null) {
            $failures[] = 'Missing flash start address';

        } elseif ($parameters->flashStartAddress > 0xFFFFFFFF) {
            $failures[] = 'Flash start address exceeds 0xFFFFFFFF - corresponding EDBG device parameter size'
                . ' is 32 bits';
        }

        if ($parameters->ramStartAddress === null) {
            $failures[] = 'Missing RAM start address';

        } elseif ($parameters->ramStartAddress > 0xFFFF) {
            $failures[] = 'RAM start address exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->eepromSize === null) {
            $failures[] = 'Missing EEPROM size';

        } elseif ($parameters->eepromSize > 0xFFFF) {
            $failures[] = 'EEPROM size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->eepromPageSize === null) {
            $failures[] = 'Missing EEPROM page size';

        } elseif ($parameters->eepromPageSize > 0xFF) {
            $failures[] = 'EEPROM page size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->eepromAddressRegisterHigh === null) {
            $failures[] = 'Missing EEARH address';

        } elseif ($parameters->eepromAddressRegisterHigh > 0xFF) {
            $failures[] = 'EEARH address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->eepromAddressRegisterLow === null) {
            $failures[] = 'Missing EEARL address';

        } elseif ($parameters->eepromAddressRegisterLow > 0xFF) {
            $failures[] = 'EEARL address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->eepromControlRegisterAddress === null) {
            $failures[] = 'Missing EECR address';

        } elseif ($parameters->eepromControlRegisterAddress > 0xFF) {
            $failures[] = 'EECR address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->eepromDataRegisterAddress === null) {
            $failures[] = 'Missing EEDR address';

        } elseif ($parameters->eepromDataRegisterAddress > 0xFF) {
            $failures[] = 'EEDR address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->ocdRevision === null) {
            $failures[] = 'Missing OCD revision';

        } elseif ($parameters->ocdRevision > 0xFF) {
            $failures[] = 'OCD revision size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->ocdDataRegister === null) {
            $failures[] = 'Missing OCDR address';

        } elseif ($parameters->ocdDataRegister > 0xFF) {
            $failures[] = 'OCDR address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->spmcRegisterStartAddress === null) {
            $failures[] = 'Missing SPMCR start address';

        } elseif ($parameters->spmcRegisterStartAddress > 0xFF) {
            $failures[] = 'SPMCR address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->osccalAddress === null) {
            $failures[] = 'Missing OSCCAL register address';

        } elseif ($parameters->osccalAddress > 0xFF) {
            $failures[] = 'OSCCALR address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        return $failures;
    }

    private function validateIspParameters(IspParameters $parameters): array
    {
        $failures = [];

        if ($parameters->programModeTimeout === null) {
            $failures[] = 'Missing ispenterprogmode_timeout ISP parameter';
        }

        if ($parameters->programModeStabilizationDelay === null) {
            $failures[] = 'Missing ispenterprogmode_stabdelay ISP parameter';
        }

        if ($parameters->programModeCommandExecutionDelay === null) {
            $failures[] = 'Missing ispenterprogmode_cmdexedelay ISP parameter';
        }

        if ($parameters->programModeSyncLoops === null) {
            $failures[] = 'Missing ispenterprogmode_synchloops ISP parameter';
        }

        if ($parameters->programModeByteDelay === null) {
            $failures[] = 'Missing ispenterprogmode_bytedelay ISP parameter';
        }

        if ($parameters->programModePollValue === null) {
            $failures[] = 'Missing ispenterprogmode_pollvalue ISP parameter';
        }

        if ($parameters->programModePollIndex === null) {
            $failures[] = 'Missing ispenterprogmode_pollindex ISP parameter';
        }

        if ($parameters->programModePreDelay === null) {
            $failures[] = 'Missing ispleaveprogmode_predelay ISP parameter';
        }

        if ($parameters->programModePostDelay === null) {
            $failures[] = 'Missing ispleaveprogmode_postdelay ISP parameter';
        }

        if ($parameters->readSignaturePollIndex === null) {
            $failures[] = 'Missing ispreadsign_pollindex ISP parameter';
        }

        if ($parameters->readFusePollIndex === null) {
            $failures[] = 'Missing ispreadfuse_pollindex ISP parameter';
        }

        if ($parameters->readLockPollIndex === null) {
            $failures[] = 'Missing ispreadlock_pollindex ISP parameter';
        }

        return $failures;
    }

    private function validateJtagParameters(JtagParameters $parameters): array
    {
        $failures = [];

        if ($parameters->flashPageSize === null) {
            $failures[] = 'Missing flash page size';

        } elseif ($parameters->flashPageSize > 0xFFFF) {
            $failures[] = 'Flash page size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->flashSize === null) {
            $failures[] = 'Missing flash size';

        } elseif ($parameters->flashSize > 0xFFFFFFFF) {
            $failures[] = 'Flash size exceeds 0xFFFFFFFF - corresponding EDBG device parameter size is 32 bits';
        }

        if ($parameters->flashStartAddress === null) {
            $failures[] = 'Missing flash start address';

        } elseif ($parameters->flashStartAddress > 0xFFFFFFFF) {
            $failures[] = 'Flash start address exceeds 0xFFFFFFFF - corresponding EDBG device parameter size'
                . ' is 32 bits';
        }

        if ($parameters->ramStartAddress === null) {
            $failures[] = 'Missing RAM start address';

        } elseif ($parameters->ramStartAddress > 0xFFFF) {
            $failures[] = 'RAM start address exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->eepromSize === null) {
            $failures[] = 'Missing EEPROM size';

        } elseif ($parameters->eepromSize > 0xFFFF) {
            $failures[] = 'EEPROM size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->eepromPageSize === null) {
            $failures[] = 'Missing EEPROM page size';

        } elseif ($parameters->eepromPageSize > 0xFF) {
            $failures[] = 'EEPROM page size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->eepromAddressRegisterHigh === null) {
            $failures[] = 'Missing EEARH address';

        } elseif ($parameters->eepromAddressRegisterHigh > 0xFF) {
            $failures[] = 'EEARH address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->eepromAddressRegisterLow === null) {
            $failures[] = 'Missing EEARL address';

        } elseif ($parameters->eepromAddressRegisterLow > 0xFF) {
            $failures[] = 'EEARL address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->eepromControlRegisterAddress === null) {
            $failures[] = 'Missing EECR address';

        } elseif ($parameters->eepromControlRegisterAddress > 0xFF) {
            $failures[] = 'EECR address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->eepromDataRegisterAddress === null) {
            $failures[] = 'Missing EEDR address';

        } elseif ($parameters->eepromDataRegisterAddress > 0xFF) {
            $failures[] = 'EEDR address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->ocdRevision === null) {
            $failures[] = 'Missing OCD revision';

        } elseif ($parameters->ocdRevision > 0xFF) {
            $failures[] = 'OCD revision size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->ocdDataRegister === null) {
            $failures[] = 'Missing OCDR address';

        } elseif ($parameters->ocdDataRegister > 0xFF) {
            $failures[] = 'OCDR address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->spmcRegisterStartAddress === null) {
            $failures[] = 'Missing SPMCR start address';

        } elseif ($parameters->spmcRegisterStartAddress > 0xFF) {
            $failures[] = 'SPMCR address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->osccalAddress === null) {
            $failures[] = 'Missing OSCCAL register address';

        } elseif ($parameters->osccalAddress > 0xFF) {
            $failures[] = 'OSCCALR address size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        return $failures;
    }

    private function validatePdiParameters(PdiParameters $parameters): array
    {
        $failures = [];

        if ($parameters->appSectionOffset === null) {
            $failures[] = 'Missing app section PDI offset';

        } elseif ($parameters->appSectionOffset > 0xFFFFFFFF) {
            $failures[] = 'App section offset exceeds 0xFFFFFFFF - corresponding EDBG device parameter size is 32 bits';
        }

        if ($parameters->bootSectionOffset === null) {
            $failures[] = 'Missing boot section PDI offset';

        } elseif ($parameters->bootSectionOffset > 0xFFFFFFFF) {
            $failures[] = 'Boot section offset exceeds 0xFFFFFFFF - corresponding EDBG device parameter size'
                . ' is 32 bits';
        }

        if ($parameters->eepromOffset === null) {
            $failures[] = 'Missing EEPROM PDI offset';

        } elseif ($parameters->eepromOffset > 0xFFFFFFFF) {
            $failures[] = 'EEPROM PDI offset exceeds 0xFFFFFFFF - corresponding EDBG device parameter size is 32 bits';
        }

        if ($parameters->fuseRegistersOffset === null) {
            $failures[] = 'Missing fuse PDI offset';

        } elseif ($parameters->fuseRegistersOffset > 0xFFFFFFFF) {
            $failures[] = 'Fuse register offset exceeds 0xFFFFFFFF - corresponding EDBG device parameter size'
                . ' is 32 bits';
        }

        if ($parameters->lockRegistersOffset === null) {
            $failures[] = 'Missing lock registers PDI offset';

        } elseif ($parameters->lockRegistersOffset > 0xFFFFFFFF) {
            $failures[] = 'Lock register offset exceeds 0xFFFFFFFF - corresponding EDBG device parameter size'
                . ' is 32 bits';
        }

        if ($parameters->userSignaturesOffset === null) {
            $failures[] = 'Missing user signatures PDI offset';

        } elseif ($parameters->userSignaturesOffset > 0xFFFFFFFF) {
            $failures[] = 'User signature offset exceeds 0xFFFFFFFF - corresponding EDBG device parameter size'
                . ' is 32 bits';
        }

        if ($parameters->productionSignaturesOffset === null) {
            $failures[] = 'Missing product signatures PDI offset';

        } elseif ($parameters->productionSignaturesOffset > 0xFFFFFFFF) {
            $failures[] = 'Prod signature offset exceeds 0xFFFFFFFF - corresponding EDBG device parameter size'
                . ' is 32 bits';
        }

        if ($parameters->ramOffset === null) {
            $failures[] = 'Missing datamem PDI offset';

        } elseif ($parameters->ramOffset > 0xFFFFFFFF) {
            $failures[] = 'RAM offset exceeds 0xFFFFFFFF - corresponding EDBG device parameter size is 32 bits';
        }

        if ($parameters->appSectionSize === null) {
            $failures[] = 'Missing app section size';

        } elseif ($parameters->appSectionSize > 0xFFFFFFFF) {
            $failures[] = 'App section size exceeds 0xFFFFFFFF - corresponding EDBG device parameter size is 32 bits';
        }

        if ($parameters->bootSectionSize === null) {
            $failures[] = 'Missing boot section size';

        } elseif ($parameters->bootSectionSize > 0xFFFF) {
            $failures[] = 'Boot section size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->flashPageSize === null) {
            $failures[] = 'Missing flash page size';

        } elseif ($parameters->flashPageSize > 0xFFFF) {
            $failures[] = 'Flash page size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->eepromSize === null) {
            $failures[] = 'Missing EEPROM size';

        } elseif ($parameters->eepromSize > 0xFFFF) {
            $failures[] = 'EEPROM size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->eepromPageSize === null) {
            $failures[] = 'Missing EEPROM page size';

        } elseif ($parameters->eepromPageSize > 0xFF) {
            $failures[] = 'EEPROM page size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->flashPageSize === null) {
            $failures[] = 'Missing flash page size';

        } elseif ($parameters->flashPageSize > 0xFFFF) {
            $failures[] = 'Flash page size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->nvmModuleBaseAddress === null) {
            $failures[] = 'Missing NVM module base address';

        } elseif ($parameters->nvmModuleBaseAddress > 0xFFFF) {
            $failures[] = 'NVM module address size exceeds 0xFFFF - corresponding EDBG device parameter size'
                . ' is 16 bits';
        }

        if ($parameters->signatureOffset === null) {
            $failures[] = 'Missing signature PDI offset';

        } elseif ($parameters->signatureOffset > 0xFFFF) {
            $failures[] = 'Signature offset size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        return $failures;
    }

    private function validateUpdiParameters(UpdiParameters $parameters): array
    {
        $failures = [];

        if ($parameters->programMemoryOffset === null) {
            $failures[] = 'Missing UPDI program memory offset';

        } elseif ($parameters->programMemoryOffset > 0xFFFFFF) {
            /*
             * Due to size constraints of EDBG AVR8 parameters for UPDI sessions, the program memory offset must
             * fit into a 24-bit integer.
             */
            $failures[] = 'Program memory offset exceeds 0xFFFFFF - corresponding EDBG device parameter size'
                . ' is 24 bits';
        }

        if ($parameters->flashPageSize === null) {
            $failures[] = 'Missing flash page size';

        } elseif ($parameters->flashPageSize > 0xFFFF) {
            $failures[] = 'Flash page size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->eepromPageSize === null) {
            $failures[] = 'Missing EEPROM page size';

        } elseif ($parameters->eepromPageSize > 0xFF) {
            $failures[] = 'EEPROM page size exceeds 0xFF - corresponding EDBG device parameter size is 8 bits';
        }

        if ($parameters->nvmModuleBaseAddress === null) {
            $failures[] = 'Missing NVM base address';

        } elseif ($parameters->nvmModuleBaseAddress > 0xFFFF) {
            $failures[] = 'NVM module address size exceeds 0xFFFF - corresponding EDBG device parameter size'
                . ' is 16 bits';
        }

        if ($parameters->ocdBaseAddress === null) {
            $failures[] = 'Missing OCD base address';

        } elseif ($parameters->ocdBaseAddress > 0xFFFF) {
            $failures[] = 'OCD base address size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->flashSize === null) {
            $failures[] = 'Missing flash size';

        } elseif ($parameters->flashSize > 0xFFFFFFFF) {
            $failures[] = 'Flash size exceeds 0xFFFFFFFF - corresponding EDBG device parameter size is 32 bits';
        }

        if ($parameters->eepromSize === null) {
            $failures[] = 'Missing EEPROM size';

        } elseif ($parameters->eepromSize > 0xFFFF) {
            $failures[] = 'EEPROM size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->eepromStartAddress === null) {
            $failures[] = 'Missing EEPROM start address';

        } elseif ($parameters->eepromStartAddress > 0xFFFF) {
            $failures[] = 'EEPROM start address size exceeds 0xFFFF - corresponding EDBG device parameter size'
                . ' is 16 bits';
        }

        if ($parameters->signatureSegmentStartAddress === null) {
            $failures[] = 'Missing signature segment start address';

        } elseif ($parameters->signatureSegmentStartAddress > 0xFFFF) {
            $failures[] = 'Signature segment start address size exceeds 0xFFFF - corresponding EDBG device parameter'
                . ' size is 16 bits';
        }

        if ($parameters->fuseSegmentStartAddress === null) {
            $failures[] = 'Missing fuses segment start address';

        } elseif ($parameters->fuseSegmentStartAddress > 0xFFFF) {
            $failures[] = 'Fuse segment start address size exceeds 0xFFFF - corresponding EDBG device parameter size'
                . ' is 16 bits';
        }

        if ($parameters->fuseSegmentSize === null) {
            $failures[] = 'Missing fuse segment size';

        } elseif ($parameters->fuseSegmentSize > 0xFFFF) {
            $failures[] = 'Fuse segment size exceeds 0xFFFF - corresponding EDBG device parameter size is 16 bits';
        }

        if ($parameters->lockbitsSegmentStartAddress === null) {
            $failures[] = 'Missing lockbits segment start address';

        } elseif ($parameters->lockbitsSegmentStartAddress > 0xFFFF) {
            $failures[] = 'Lockbit segment start address size exceeds 0xFFFF - corresponding EDBG device parameter'
                . ' size is 16 bits';
        }

        return $failures;
    }
}
