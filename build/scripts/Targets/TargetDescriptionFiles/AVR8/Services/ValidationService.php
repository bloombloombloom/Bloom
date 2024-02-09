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
        }

        if ($parameters->flashSize === null) {
            $failures[] = 'Missing flash size';
        }

        if ($parameters->flashStartAddress === null) {
            $failures[] = 'Missing flash start address';
        }

        if ($parameters->ramStartAddress === null) {
            $failures[] = 'Missing ram start address';
        }

        if ($parameters->eepromSize === null) {
            $failures[] = 'Missing eeprom size';
        }

        if ($parameters->eepromPageSize === null) {
            $failures[] = 'Missing eeprom page size';
        }

        if ($parameters->eepromAddressRegisterHigh === null) {
            $failures[] = 'Missing EEARH register address';
        }

        if ($parameters->eepromAddressRegisterLow === null) {
            $failures[] = 'Missing EEARL register address';
        }

        if ($parameters->eepromControlRegisterAddress === null) {
            $failures[] = 'Missing EECR register address';
        }

        if ($parameters->eepromDataRegisterAddress === null) {
            $failures[] = 'Missing EEDR register address';
        }

        if ($parameters->ocdRevision === null) {
            $failures[] = 'Missing OCD revision';
        }

        if ($parameters->ocdDataRegister === null) {
            $failures[] = 'Missing OCD data register address';
        }

        if ($parameters->spmcRegisterStartAddress === null) {
            $failures[] = 'Missing store program memory control register start address';
        }

        if ($parameters->osccalAddress === null) {
            $failures[] = 'Missing oscillator calibration register address';
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
        }

        if ($parameters->flashSize === null) {
            $failures[] = 'Missing flash size';
        }

        if ($parameters->flashStartAddress === null) {
            $failures[] = 'Missing flash start address';
        }

        if (empty($parameters->bootSections)) {
            $failures[] = 'Missing boot sections';
        }

        if ($parameters->ramStartAddress === null) {
            $failures[] = 'Missing ram start address';
        }

        if ($parameters->eepromSize === null) {
            $failures[] = 'Missing eeprom size';
        }

        if ($parameters->eepromPageSize === null) {
            $failures[] = 'Missing eeprom page size';
        }

        if ($parameters->eepromAddressRegisterHigh === null) {
            $failures[] = 'Missing EEARH register address';
        }

        if ($parameters->eepromAddressRegisterLow === null) {
            $failures[] = 'Missing EEARL register address';
        }

        if ($parameters->eepromControlRegisterAddress === null) {
            $failures[] = 'Missing EECR register address';
        }

        if ($parameters->eepromDataRegisterAddress === null) {
            $failures[] = 'Missing EEDR register address';
        }

        if ($parameters->ocdRevision === null) {
            $failures[] = 'Missing OCD revision';
        }

        if ($parameters->ocdDataRegister === null) {
            $failures[] = 'Missing OCD data register address';
        }

        if ($parameters->spmcRegisterStartAddress === null) {
            $failures[] = 'Missing store program memory control register start address';
        }

        if ($parameters->osccalAddress === null) {
            $failures[] = 'Missing oscillator calibration register address';
        }

        return $failures;
    }

    private function validatePdiParameters(PdiParameters $parameters): array
    {
        $failures = [];

        if ($parameters->appSectionOffset === null) {
            $failures[] = 'Missing app section PDI offset';
        }

        if ($parameters->bootSectionOffset === null) {
            $failures[] = 'Missing boot section PDI offset';
        }

        if ($parameters->eepromOffset === null) {
            $failures[] = 'Missing eeprom PDI offset';
        }

        if ($parameters->fuseRegistersOffset === null) {
            $failures[] = 'Missing fuse PDI offset';
        }

        if ($parameters->lockRegistersOffset === null) {
            $failures[] = 'Missing lock registers PDI offset';
        }

        if ($parameters->userSignaturesOffset === null) {
            $failures[] = 'Missing user signatures PDI offset';
        }

        if ($parameters->productSignaturesOffset === null) {
            $failures[] = 'Missing product signatures PDI offset';
        }

        if ($parameters->ramOffset === null) {
            $failures[] = 'Missing datamem PDI offset';
        }

        if ($parameters->appSectionSize === null) {
            $failures[] = 'Missing app section size';
        }

        if ($parameters->bootSectionSize === null) {
            $failures[] = 'Missing boot section size';
        }

        if ($parameters->flashPageSize === null) {
            $failures[] = 'Missing flash page size';
        }

        if ($parameters->eepromSize === null) {
            $failures[] = 'Missing EEPROM size';
        }

        if ($parameters->flashPageSize === null) {
            $failures[] = 'Missing flash page size';
        }

        if ($parameters->nvmModuleBaseAddress === null) {
            $failures[] = 'Missing NVM module base address';
        }

        if ($parameters->signatureOffset === null) {
            $failures[] = 'Missing signature PDI offset';
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
            $failures[] = 'UPDI program memory offset exceeds maximum value for 24-bit unsigned integer';
        }

        if ($parameters->flashPageSize === null) {
            $failures[] = 'Missing flash page size';

        } elseif ($parameters->flashPageSize > 0xFFFF) {
            $failures[] = 'Flash page size exceeds maximum value for 16-bit unsigned integer';
        }

        if ($parameters->eepromPageSize === null) {
            $failures[] = 'Missing EEPROM page size';
        }

        if ($parameters->nvmModuleBaseAddress === null) {
            $failures[] = 'Missing NVM base address';
        }

        if ($parameters->ocdBaseAddress === null) {
            $failures[] = 'Missing OCD base address';

        } elseif ($parameters->ocdBaseAddress > 0xFFFF) {
            $failures[] = 'UPDI OCD base address exceeds maximum value for 16-bit unsigned integer';
        }

        if ($parameters->flashSize === null) {
            $failures[] = 'Missing flash size';
        }

        if ($parameters->eepromSize === null) {
            $failures[] = 'Missing EEPROM size';
        }

        if ($parameters->eepromStartAddress === null) {
            $failures[] = 'Missing EEPROM start address';
        }

        if ($parameters->signatureSegmentStartAddress === null) {
            $failures[] = 'Missing signature segment start address';
        }

        if ($parameters->fuseSegmentStartAddress === null) {
            $failures[] = 'Missing fuses segment start address';
        }

        if ($parameters->fuseSegmentSize === null) {
            $failures[] = 'Missing fuse segment size';
        }

        if ($parameters->lockbitsSegmentStartAddress === null) {
            $failures[] = 'Missing lockbits segment start address';
        }

        return $failures;
    }
}
