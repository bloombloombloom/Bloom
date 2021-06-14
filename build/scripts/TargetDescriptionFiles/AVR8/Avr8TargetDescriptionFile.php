<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

use Bloom\BuildScripts\TargetDescriptionFiles\TargetDescriptionFile;

require_once __DIR__ . "/../TargetDescriptionFile.php";
require_once __DIR__ . "/Signature.php";

class Avr8TargetDescriptionFile extends TargetDescriptionFile
{
    const AVR8_FAMILY_MEGA = 'MEGA';
    const AVR8_FAMILY_TINY = 'TINY';
    const AVR8_FAMILY_XMEGA = 'XMEGA';
    const AVR8_FAMILY_DB = 'DB';
    const AVR8_FAMILY_DA = 'DA';
    const AVR8_FAMILY_DD = 'DD';
    const AVR8_FAMILY_OTHER = 'OTHER';

    const AVR8_PHYSICAL_INTERFACE_JTAG = 'JTAG';
    const AVR8_PHYSICAL_INTERFACE_PDI = 'PDI';
    const AVR8_PHYSICAL_INTERFACE_UPDI = 'UPDI';
    const AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE = 'DEBUG_WIRE';

    public ?Signature $signature = null;
    public ?string $family = null;
    public array $debugPhysicalInterface = [];

    // Target params
    public ?int $bootSectionStartAddress = null;
    public ?int $gpRegisterStartAddress = null;
    public ?int $gpRegisterSize = null;
    public ?int $flashPageSize = null;
    public ?int $flashSize = null;
    public ?int $flashStartAddress = null;
    public ?int $ramStartAddress = null;
    public ?int $ramSize = null;
    public ?int $eepromSize = null;
    public ?int $eepromPageSize = null;
    public ?int $eepromAddressRegisterHigh = null;
    public ?int $eepromAddressRegisterLow = null;
    public ?int $eepromDataRegisterAddress = null;
    public ?int $eepromControlRegisterAddress = null;
    public ?int $ocdRevision = null;
    public ?int $ocdDataRegister = null;
    public ?int $statusRegisterStartAddress = null;
    public ?int $statusRegisterSize = null;
    public ?int $stackPointerRegisterStartAddress = null;
    public ?int $stackPointerRegisterSize = null;
    public ?int $spmcRegisterStartAddress = null;
    public ?int $osccalAddress = null;

    // XMega/PDI specific target params
    public ?int $appSectionPdiOffset = null;
    public ?int $bootSectionSize = null;
    public ?int $bootSectionPdiOffset = null;
    public ?int $eepromPdiOffset = null;
    public ?int $ramPdiOffset = null;
    public ?int $fuseRegistersPdiOffset = null;
    public ?int $lockRegistersPdiOffset = null;
    public ?int $userSignaturesPdiOffset = null;
    public ?int $productSignaturesPdiOffset = null;
    public ?int $nvmBaseAddress = null;

    protected function init()
    {
        parent::init();

        $device = $this->xml->device;
        if (empty($device)) {
            return;
        }

        $deviceAttributes = $device->attributes();

        if (!empty($deviceAttributes['family'])) {
            if (stristr($deviceAttributes['family'], 'xmega') !== false) {
                $this->family = self::AVR8_FAMILY_XMEGA;

            } else if (stristr($deviceAttributes['family'], 'tiny') !== false) {
                $this->family = self::AVR8_FAMILY_TINY;

            } else if (stristr($deviceAttributes['family'], 'mega') !== false) {
                $this->family = self::AVR8_FAMILY_MEGA;

            } else if (strtolower(trim($deviceAttributes['family'])) == 'avr da') {
                $this->family = self::AVR8_FAMILY_DA;

            } else if (strtolower(trim($deviceAttributes['family'])) == 'avr db') {
                $this->family = self::AVR8_FAMILY_DB;

            } else if (strtolower(trim($deviceAttributes['family'])) == 'avr dd') {
                $this->family = self::AVR8_FAMILY_DD;

            } else {
                $this->family = self::AVR8_FAMILY_OTHER;
            }
        }

        if (isset($this->physicalInterfacesByName['debugwire'])) {
            $this->debugPhysicalInterface[] = self::AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE;
        }

        if (isset($this->physicalInterfacesByName['updi'])) {
            $this->debugPhysicalInterface[] = self::AVR8_PHYSICAL_INTERFACE_UPDI;

        } else if (isset($this->physicalInterfacesByName['pdi'])) {
            $this->debugPhysicalInterface[] = self::AVR8_PHYSICAL_INTERFACE_PDI;
        }

        if (isset($this->physicalInterfacesByName['jtag'])) {
            $this->debugPhysicalInterface[] = self::AVR8_PHYSICAL_INTERFACE_JTAG;
        }

        $signaturePropertyGroup = $this->propertyGroupsByName['signatures'] ?? null;
        if (!empty($signaturePropertyGroup)) {
            $this->signature = new Signature();

            if (isset($signaturePropertyGroup->propertiesMappedByName['signature0'])) {
                $this->signature->byteZero = $this->rawValueToInt(
                    $signaturePropertyGroup->propertiesMappedByName['signature0']->value
                );
            }

            if (isset($signaturePropertyGroup->propertiesMappedByName['signature1'])) {
                $this->signature->byteOne = $this->rawValueToInt(
                    $signaturePropertyGroup->propertiesMappedByName['signature1']->value
                );
            }

            if (isset($signaturePropertyGroup->propertiesMappedByName['signature2'])) {
                $this->signature->byteTwo = $this->rawValueToInt(
                    $signaturePropertyGroup->propertiesMappedByName['signature2']->value
                );
            }
        }

        $progAddressSpace = $this->addressSpacesById['prog'] ?? null;
        if (!empty($progAddressSpace)) {
            $flashMemorySegment = $progAddressSpace->memorySegmentsByTypeAndName['flash']['flash']
                ?? $progAddressSpace->memorySegmentsByTypeAndName['flash']['app_section']
                ?? $progAddressSpace->memorySegmentsByTypeAndName['flash']['progmem'] ?? null;
            $bootSectionMemorySegment = $progAddressSpace->memorySegmentsByTypeAndName['flash']['boot_section_1']
                ?? $progAddressSpace->memorySegmentsByTypeAndName['flash']['boot_section'] ?? null;

            if (!empty($flashMemorySegment)) {
                $this->flashSize = $flashMemorySegment->size;
                $this->flashStartAddress = $flashMemorySegment->startAddress;
                $this->flashPageSize = $flashMemorySegment->pageSize;
            }

            if (!empty($bootSectionMemorySegment)) {
                $this->bootSectionSize = $bootSectionMemorySegment->size;
                $this->bootSectionStartAddress = $bootSectionMemorySegment->startAddress;
            }
        }

        $dataAddressSpace = $this->addressSpacesById['data'] ?? null;
        if (!empty($dataAddressSpace)) {
            $ramMemorySegments = $dataAddressSpace->memorySegmentsByTypeAndName['ram'] ?? null;
            $registerMemorySegments = $dataAddressSpace->memorySegmentsByTypeAndName['regs'] ?? null;

            if (!empty($ramMemorySegments)) {
                $ramMemorySegment = reset($ramMemorySegments);
                $this->ramSize = $ramMemorySegment->size;
                $this->ramStartAddress = $ramMemorySegment->startAddress;
            }

            if (!empty($registerMemorySegments)) {
                $registerMemorySegment = reset($registerMemorySegments);
                $this->gpRegisterSize = $registerMemorySegment->size;
                $this->gpRegisterStartAddress = $registerMemorySegment->startAddress;
            }

            if (isset($dataAddressSpace->memorySegmentsByTypeAndName['eeprom']['eeprom'])) {
                $eepromMemorySegment = $dataAddressSpace->memorySegmentsByTypeAndName['eeprom']['eeprom'];
                $this->eepromSize = $eepromMemorySegment->size;
                $this->eepromPageSize = $eepromMemorySegment->pageSize;
            }
        }

        if ((is_null($this->eepromSize) || is_null($this->eepromPageSize))
            && isset($this->addressSpacesById['eeprom'])
        ) {
            $eepromAddressSpace = $this->addressSpacesById['eeprom'];
            $eepromMemorySegments = $eepromAddressSpace->memorySegmentsByTypeAndName['eeprom'] ?? null;

            if (!empty($eepromMemorySegments)) {
                $eepromMemorySegment = reset($eepromMemorySegments);
                $this->eepromSize = $eepromMemorySegment->size;
                $this->eepromPageSize = $eepromMemorySegment->pageSize;
            }
        }

        if (isset($this->propertyGroupsByName['ocd'])) {
            $ocdProperties = $this->propertyGroupsByName['ocd']->propertiesMappedByName;

            $this->ocdRevision = isset($ocdProperties['ocd_revision']) ? (int) $ocdProperties['ocd_revision']->value : null;
            $this->ocdDataRegister = isset($ocdProperties['ocd_datareg'])
                ? $this->rawValueToInt($ocdProperties['ocd_datareg']->value) : null;
        }

        if (isset($this->modulesByName['cpu'])) {
            $cpuModule = $this->modulesByName['cpu'];

            if (isset($cpuModule->registerGroupsMappedByName['cpu'])) {
                $cpuRegisterGroup = $cpuModule->registerGroupsMappedByName['cpu'];

                if (isset($cpuRegisterGroup->registersMappedByName['sreg'])) {
                    $statusRegister = $cpuRegisterGroup->registersMappedByName['sreg'];
                    $this->statusRegisterSize = $statusRegister->size;
                    $this->statusRegisterStartAddress = $statusRegister->offset;
                }

                if (isset($cpuRegisterGroup->registersMappedByName['sp'])) {
                    $stackPointerRegister = $cpuRegisterGroup->registersMappedByName['sp'];
                    $this->stackPointerRegisterSize = $stackPointerRegister->size;
                    $this->stackPointerRegisterStartAddress = $stackPointerRegister->offset;

                } else {
                    if (isset($cpuRegisterGroup->registersMappedByName['spl'])) {
                        $stackPointerLowRegister = $cpuRegisterGroup->registersMappedByName['spl'];
                        $this->stackPointerRegisterSize = $stackPointerLowRegister->size;
                        $this->stackPointerRegisterStartAddress = $stackPointerLowRegister->offset;

                        if (isset($cpuRegisterGroup->registersMappedByName['sph'])) {
                            $stackPointerHighRegister = $cpuRegisterGroup->registersMappedByName['sph'];
                            $this->stackPointerRegisterSize += $stackPointerHighRegister->size;
                        }
                    }
                }

                if (isset($cpuRegisterGroup->registersMappedByName['spmcsr'])) {
                    $spmcsRegister = $cpuRegisterGroup->registersMappedByName['spmcsr'];
                    $this->spmcRegisterStartAddress = $spmcsRegister->offset;

                } else if (isset($cpuRegisterGroup->registersMappedByName['spmcr'])) {
                    $spmcRegister = $cpuRegisterGroup->registersMappedByName['spmcr'];
                    $this->spmcRegisterStartAddress = $spmcRegister->offset;

                } else {
                    if (isset($this->modulesByName['boot_load'])
                        && isset($this->modulesByName['boot_load']->registerGroupsMappedByName['boot_load'])
                    ) {
                        $bootLoadedModule = $this->modulesByName['boot_load'];
                        $bootLoaderRegisterGroup = $bootLoadedModule->registerGroupsMappedByName['boot_load'];

                        if (isset($bootLoaderRegisterGroup->registersMappedByName['spmcr'])) {
                            $spmcRegister = $bootLoaderRegisterGroup->registersMappedByName['spmcr'];
                            $this->spmcRegisterStartAddress = $spmcRegister->offset;

                        } else if (isset($bootLoaderRegisterGroup->registersMappedByName['spmcsr'])) {
                            $spmcsRegister = $bootLoaderRegisterGroup->registersMappedByName['spmcsr'];
                            $this->spmcRegisterStartAddress = $spmcsRegister->offset;
                        }
                    }
                }

                $osccalRegister = $cpuRegisterGroup->registersMappedByName['osccal']
                    ?? $cpuRegisterGroup->registersMappedByName['osccal0']
                    ?? $cpuRegisterGroup->registersMappedByName['osccal1']
                    ?? $cpuRegisterGroup->registersMappedByName['fosccal']
                    ?? $cpuRegisterGroup->registersMappedByName['sosccala'] ?? null;

                if (!is_null($osccalRegister)) {
                    $this->osccalAddress = $osccalRegister->offset;
                }
            }
        }

        if (isset($this->modulesByName['eeprom'])) {
            $eepromModule = $this->modulesByName['eeprom'];

            if (isset($eepromModule->registerGroupsMappedByName['eeprom'])) {
                $eepromRegisterGroup = $eepromModule->registerGroupsMappedByName['eeprom'];

                if (isset($eepromRegisterGroup->registersMappedByName['eear'])) {
                    $eearRegister = $eepromRegisterGroup->registersMappedByName['eear'];
                    $this->eepromAddressRegisterLow = $eearRegister->offset;
                    $this->eepromAddressRegisterHigh = ($eearRegister->size == 2)
                        ? $eearRegister->offset + 1 : $eearRegister->offset;

                } else {
                    if (isset($eepromRegisterGroup->registersMappedByName['eearl'])) {
                        $eearlRegister = $eepromRegisterGroup->registersMappedByName['eearl'];
                        $this->eepromAddressRegisterLow = $eearlRegister->offset;

                        if (isset($eepromRegisterGroup->registersMappedByName['eearh'])) {
                            $eearhRegister = $eepromRegisterGroup->registersMappedByName['eearh'];
                            $this->eepromAddressRegisterHigh = $eearhRegister->offset;

                        } else {
                            $this->eepromAddressRegisterHigh = $eearlRegister->offset;
                        }

                    }
                }

                if (isset($eepromRegisterGroup->registersMappedByName['eedr'])) {
                    $eedrRegister = $eepromRegisterGroup->registersMappedByName['eedr'];
                    $this->eepromDataRegisterAddress = $eedrRegister->offset;
                }

                if (isset($eepromRegisterGroup->registersMappedByName['eecr'])) {
                    $eecrRegister = $eepromRegisterGroup->registersMappedByName['eecr'];
                    $this->eepromControlRegisterAddress = $eecrRegister->offset;
                }
            }
        }

        if (isset($this->propertyGroupsByName['pdi_interface'])) {
            $pdiInterfacePropertyGroup = $this->propertyGroupsByName['pdi_interface'];
            $pdiInterfacePropertiesByName = $pdiInterfacePropertyGroup->propertiesMappedByName;

            if (isset($pdiInterfacePropertiesByName['app_section_offset'])) {
                $this->appSectionPdiOffset = isset($pdiInterfacePropertiesByName['app_section_offset']->value)
                    ? $this->rawValueToInt($pdiInterfacePropertiesByName['app_section_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['boot_section_offset'])) {
                $this->bootSectionPdiOffset = isset($pdiInterfacePropertiesByName['boot_section_offset']->value)
                    ? $this->rawValueToInt($pdiInterfacePropertiesByName['boot_section_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['datamem_offset'])) {
                $this->ramPdiOffset = isset($pdiInterfacePropertiesByName['datamem_offset']->value)
                    ? $this->rawValueToInt($pdiInterfacePropertiesByName['datamem_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['eeprom_offset'])) {
                $this->eepromPdiOffset = isset($pdiInterfacePropertiesByName['eeprom_offset']->value)
                    ? $this->rawValueToInt($pdiInterfacePropertiesByName['eeprom_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['user_signatures_offset'])) {
                $this->userSignaturesPdiOffset = isset($pdiInterfacePropertiesByName['user_signatures_offset']->value)
                    ? $this->rawValueToInt($pdiInterfacePropertiesByName['user_signatures_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['prod_signatures_offset'])) {
                $this->productSignaturesPdiOffset = isset($pdiInterfacePropertiesByName['prod_signatures_offset']->value)
                    ? $this->rawValueToInt($pdiInterfacePropertiesByName['prod_signatures_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['fuse_registers_offset'])) {
                $this->fuseRegistersPdiOffset = isset($pdiInterfacePropertiesByName['fuse_registers_offset']->value)
                    ? $this->rawValueToInt($pdiInterfacePropertiesByName['fuse_registers_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['lock_registers_offset'])) {
                $this->lockRegistersPdiOffset = isset($pdiInterfacePropertiesByName['lock_registers_offset']->value)
                    ? $this->rawValueToInt($pdiInterfacePropertiesByName['lock_registers_offset']->value) : null;
            }

            if (isset($this->peripheralModulesByName['nvm'])) {
                $nvmModule = $this->peripheralModulesByName['nvm'];

                if (isset($nvmModule->instancesMappedByName['nvm'])) {
                    $nvmInstance = $nvmModule->instancesMappedByName['nvm'];

                    if (isset($nvmInstance->registerGroupsMappedByName['nvm'])) {
                        $this->nvmBaseAddress = $nvmInstance->registerGroupsMappedByName['nvm']->offset;
                    }
                }
            }
        }
    }

    public function validate(): array
    {
        $failures = parent::validate();

        if (is_null($this->signature)
            || is_null($this->signature->byteZero)
            || is_null($this->signature->byteOne)
            || is_null($this->signature->byteTwo)
        ) {
            $failures[] = "Missing or incomplete AVR signature.";
        }

        if (is_null($this->debugPhysicalInterface)) {
            $failures[] = 'Target does not support any known AVR8 debug interface - the TDF will need to be deleted.'
                . ' Aborting validation.';
            return $failures;
        }

        if (is_null($this->family) || $this->family == self::AVR8_FAMILY_OTHER) {
            $failures[] = 'Unknown AVR8 family';
        }

        if (is_null($this->stackPointerRegisterStartAddress)) {
            $failures[] = 'Missing stack pointer register start address.';
        }

        if (is_null($this->stackPointerRegisterSize)) {
            $failures[] = 'Missing stack pointer register size.';
        }

        if (is_null($this->statusRegisterStartAddress)) {
            $failures[] = 'Missing status register start address.';
        }

        if (is_null($this->statusRegisterSize)) {
            $failures[] = 'Missing status register size.';
        }

        if (is_null($this->flashSize)) {
            $failures[] = 'Missing flash size.';
        }

        if (is_null($this->flashPageSize)) {
            $failures[] = 'Missing flash page size.';
        }

        if (is_null($this->flashStartAddress)) {
            $failures[] = 'Missing flash start address.';
        }

        if (is_null($this->ramStartAddress)) {
            $failures[] = 'Missing ram start address.';
        }

        if (is_null($this->eepromSize)) {
            $failures[] = 'Missing eeprom size.';
        }

        if (is_null($this->eepromPageSize)) {
            $failures[] = 'Missing eeprom page size.';
        }

        if (in_array(self::AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE, $this->debugPhysicalInterface)
            || (
                in_array(self::AVR8_PHYSICAL_INTERFACE_JTAG, $this->debugPhysicalInterface)
                && $this->family == self::AVR8_FAMILY_MEGA
            )
        ) {
            if (is_null($this->ocdRevision)) {
                $failures[] = 'Missing OCD revision.';
            }

            if (is_null($this->ocdDataRegister)) {
                $failures[] = 'Missing OCD data register address.';
            }

            if (is_null($this->spmcRegisterStartAddress)) {
                $failures[] = 'Missing store program memory control register start address.';
            }

            if (is_null($this->osccalAddress)) {
                $failures[] = 'Missing oscillator calibration register address.';
            }
        }

        if ($this->debugPhysicalInterface == self::AVR8_PHYSICAL_INTERFACE_PDI) {
            if (is_null($this->appSectionPdiOffset)) {
                $failures[] = 'Missing app section PDI offset.';
            }

            if (is_null($this->bootSectionPdiOffset)) {
                $failures[] = 'Missing boot section PDI offset.';
            }

            if (is_null($this->ramPdiOffset)) {
                $failures[] = 'Missing datamem PDI offset.';
            }

            if (is_null($this->eepromPdiOffset)) {
                $failures[] = 'Missing eeprom PDI offset.';
            }

            if (is_null($this->userSignaturesPdiOffset)) {
                $failures[] = 'Missing user signatures PDI offset.';
            }

            if (is_null($this->productSignaturesPdiOffset)) {
                $failures[] = 'Missing product signatures PDI offset.';
            }

            if (is_null($this->lockRegistersPdiOffset)) {
                $failures[] = 'Missing lock registers PDI offset.';
            }

            if (is_null($this->nvmBaseAddress)) {
                $failures[] = 'Missing nvm start address.';
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
