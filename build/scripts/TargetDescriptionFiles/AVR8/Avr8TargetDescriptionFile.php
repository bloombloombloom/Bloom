<?php
namespace Bloom\BuildScripts\TargetDescriptionFiles\Avr8;

use Bloom\BuildScripts\TargetDescriptionFiles\TargetDescriptionFile;

require_once __DIR__ . "/../TargetDescriptionFile.php";
require_once __DIR__ . "/Signature.php";
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

    public ?Signature $signature = null;
    public ?string $family = null;
    public array $physicalInterfaces = [];
    public array $debugPhysicalInterfaces = [];

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
    public ?int $eepromStartAddress = null;
    public ?int $eepromAddressRegisterHigh = null;
    public ?int $eepromAddressRegisterLow = null;
    public ?int $eepromDataRegisterAddress = null;
    public ?int $eepromControlRegisterAddress = null;
    public ?int $ocdRevision = null;
    public ?int $ocdDataRegister = null;
    public ?int $statusRegisterStartAddress = null;
    public ?int $statusRegisterSize = null;
    public ?int $stackPointerRegisterLowAddress = null;
    public ?int $stackPointerRegisterSize = null;
    public ?int $spmcRegisterStartAddress = null;
    public ?int $osccalAddress = null;

    // XMega/PDI specific target params
    public ?int $appSectionStartAddress = null;
    public ?int $appSectionSize = null;
    public ?int $bootSectionSize = null;
    public ?int $appSectionPdiOffset = null;
    public ?int $bootSectionPdiOffset = null;
    public ?int $eepromPdiOffset = null;
    public ?int $ramPdiOffset = null;
    public ?int $fuseRegistersPdiOffset = null;
    public ?int $lockRegistersPdiOffset = null;
    public ?int $userSignaturesPdiOffset = null;
    public ?int $productSignaturesPdiOffset = null;
    public ?int $nvmModuleBaseAddress = null;
    public ?int $mcuModuleBaseAddress = null;

    // UPDI specific target params
    public ?int $ocdBaseAddress = null;
    public ?int $programMemoryUpdiStartAddress = null;
    public ?int $signatureSegmentStartAddress = null;
    public ?int $signatureSegmentSize = null;
    public ?int $fuseSegmentStartAddress = null;
    public ?int $fuseSegmentSize = null;
    public ?int $lockbitsSegmentStartAddress = null;

    // ISP parameters
    public ?int $ispProgramModeTimeout = null;
    public ?int $ispProgramModeStabilizationDelay = null;
    public ?int $ispProgramModeCommandExecutionDelay = null;
    public ?int $ispProgramModeSyncLoops = null;
    public ?int $ispProgramModeByteDelay = null;
    public ?int $ispProgramModePollValue = null;
    public ?int $ispProgramModePollIndex = null;
    public ?int $ispProgramModePreDelay = null;
    public ?int $ispProgramModePostDelay = null;
    public ?int $ispReadSignaturePollIndex = null;
    public ?int $ispReadFusePollIndex = null;
    public ?int $ispReadLockPollIndex = null;

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

            } else if (strtolower(trim($deviceAttributes['family'])) == 'avr ea') {
                $this->family = self::AVR8_FAMILY_EA;

            } else {
                $this->family = self::AVR8_FAMILY_OTHER;
            }
        }

        if (isset($this->physicalInterfacesByName['isp'])) {
            $this->physicalInterfaces[] = self::AVR8_PHYSICAL_INTERFACE_ISP;
        }

        if (isset($this->physicalInterfacesByName['debugwire'])) {
            $this->debugPhysicalInterfaces[] = self::AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE;
            $this->physicalInterfaces[] = self::AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE;
        }

        if (isset($this->physicalInterfacesByName['updi'])) {
            $this->debugPhysicalInterfaces[] = self::AVR8_PHYSICAL_INTERFACE_UPDI;
            $this->physicalInterfaces[] = self::AVR8_PHYSICAL_INTERFACE_UPDI;

        } else if (isset($this->physicalInterfacesByName['pdi'])) {
            $this->debugPhysicalInterfaces[] = self::AVR8_PHYSICAL_INTERFACE_PDI;
            $this->physicalInterfaces[] = self::AVR8_PHYSICAL_INTERFACE_PDI;
        }

        if (isset($this->physicalInterfacesByName['jtag'])) {
            $this->debugPhysicalInterfaces[] = self::AVR8_PHYSICAL_INTERFACE_JTAG;
            $this->physicalInterfaces[] = self::AVR8_PHYSICAL_INTERFACE_JTAG;
        }

        $signaturePropertyGroup = $this->propertyGroupsByName['signatures'] ?? null;
        if (!empty($signaturePropertyGroup)) {
            $this->signature = new Signature();

            if (isset($signaturePropertyGroup->propertiesMappedByName['signature0'])) {
                $this->signature->byteZero = $this->stringToInt(
                    $signaturePropertyGroup->propertiesMappedByName['signature0']->value
                );
            }

            if (isset($signaturePropertyGroup->propertiesMappedByName['signature1'])) {
                $this->signature->byteOne = $this->stringToInt(
                    $signaturePropertyGroup->propertiesMappedByName['signature1']->value
                );
            }

            if (isset($signaturePropertyGroup->propertiesMappedByName['signature2'])) {
                $this->signature->byteTwo = $this->stringToInt(
                    $signaturePropertyGroup->propertiesMappedByName['signature2']->value
                );
            }
        }

        $ispParamPropertyGroup = $this->propertyGroupsByName['isp_interface'] ?? null;
        if (!empty($ispParamPropertyGroup)) {
            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_timeout'])) {
                $this->ispProgramModeTimeout = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_timeout']->value
                );
            }

            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_stabdelay'])) {
                $this->ispProgramModeStabilizationDelay = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_stabdelay']->value
                );
            }

            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_cmdexedelay'])) {
                $this->ispProgramModeCommandExecutionDelay = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_cmdexedelay']->value
                );
            }

            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_synchloops'])) {
                $this->ispProgramModeSyncLoops = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_synchloops']->value
                );
            }

            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_bytedelay'])) {
                $this->ispProgramModeByteDelay = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_bytedelay']->value
                );
            }

            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_pollvalue'])) {
                $this->ispProgramModePollValue = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_pollvalue']->value
                );
            }

            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_pollindex'])) {
                $this->ispProgramModePollIndex = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispenterprogmode_pollindex']->value
                );
            }

            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispleaveprogmode_predelay'])) {
                $this->ispProgramModePreDelay = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispleaveprogmode_predelay']->value
                );
            }

            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispleaveprogmode_postdelay'])) {
                $this->ispProgramModePostDelay = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispleaveprogmode_postdelay']->value
                );
            }

            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispreadsign_pollindex'])) {
                $this->ispReadSignaturePollIndex = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispreadsign_pollindex']->value
                );
            }
            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispreadfuse_pollindex'])) {
                $this->ispReadFusePollIndex = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispreadfuse_pollindex']->value
                );
            }
            if (isset($ispParamPropertyGroup->propertiesMappedByName['ispreadlock_pollindex'])) {
                $this->ispReadLockPollIndex = $this->stringToInt(
                    $ispParamPropertyGroup->propertiesMappedByName['ispreadlock_pollindex']->value
                );
            }
        }

        $progAddressSpace = $this->addressSpacesById['prog'] ?? null;
        if (!empty($progAddressSpace)) {
            $this->flashSize = $progAddressSpace->size;
            $this->flashStartAddress = $progAddressSpace->startAddress;

            $firstFlashMemorySegment = reset($progAddressSpace->memorySegmentsByTypeAndName['flash']);
            $appFlashMemorySegment = $progAddressSpace->memorySegmentsByTypeAndName['flash']['app_section'] ?? null;
            $bootSectionMemorySegment = $progAddressSpace->memorySegmentsByTypeAndName['flash']['boot_section_1']
                ?? $progAddressSpace->memorySegmentsByTypeAndName['flash']['boot_section'] ?? null;

            if (!empty($firstFlashMemorySegment)) {
                $this->flashPageSize = $firstFlashMemorySegment->pageSize;
            }

            if (!empty($appFlashMemorySegment)) {
                $this->appSectionStartAddress = $appFlashMemorySegment->startAddress;
                $this->appSectionSize = $appFlashMemorySegment->size;
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
                $this->eepromStartAddress = $eepromMemorySegment->startAddress;
                $this->eepromSize = $eepromMemorySegment->size;
                $this->eepromPageSize = $eepromMemorySegment->pageSize;
            }
        }

        if (
            (is_null($this->eepromSize) || is_null($this->eepromPageSize) || is_null($this->eepromStartAddress))
            && isset($this->addressSpacesById['eeprom'])
        ) {
            $eepromAddressSpace = $this->addressSpacesById['eeprom'];
            $eepromMemorySegments = $eepromAddressSpace->memorySegmentsByTypeAndName['eeprom'] ?? null;
            $this->eepromStartAddress = $eepromAddressSpace->startAddress;

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
                ? $this->stringToInt($ocdProperties['ocd_datareg']->value) : null;
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
                    $this->stackPointerRegisterLowAddress = $stackPointerRegister->offset;

                } else {
                    if (isset($cpuRegisterGroup->registersMappedByName['spl'])) {
                        $stackPointerLowRegister = $cpuRegisterGroup->registersMappedByName['spl'];
                        $this->stackPointerRegisterSize = $stackPointerLowRegister->size;
                        $this->stackPointerRegisterLowAddress = $stackPointerLowRegister->offset;

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
                    ? $this->stringToInt($pdiInterfacePropertiesByName['app_section_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['boot_section_offset'])) {
                $this->bootSectionPdiOffset = isset($pdiInterfacePropertiesByName['boot_section_offset']->value)
                    ? $this->stringToInt($pdiInterfacePropertiesByName['boot_section_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['datamem_offset'])) {
                $this->ramPdiOffset = isset($pdiInterfacePropertiesByName['datamem_offset']->value)
                    ? $this->stringToInt($pdiInterfacePropertiesByName['datamem_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['eeprom_offset'])) {
                $this->eepromPdiOffset = isset($pdiInterfacePropertiesByName['eeprom_offset']->value)
                    ? $this->stringToInt($pdiInterfacePropertiesByName['eeprom_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['user_signatures_offset'])) {
                $this->userSignaturesPdiOffset = isset($pdiInterfacePropertiesByName['user_signatures_offset']->value)
                    ? $this->stringToInt($pdiInterfacePropertiesByName['user_signatures_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['prod_signatures_offset'])) {
                $this->productSignaturesPdiOffset = isset($pdiInterfacePropertiesByName['prod_signatures_offset']->value)
                    ? $this->stringToInt($pdiInterfacePropertiesByName['prod_signatures_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['fuse_registers_offset'])) {
                $this->fuseRegistersPdiOffset = isset($pdiInterfacePropertiesByName['fuse_registers_offset']->value)
                    ? $this->stringToInt($pdiInterfacePropertiesByName['fuse_registers_offset']->value) : null;
            }

            if (isset($pdiInterfacePropertiesByName['lock_registers_offset'])) {
                $this->lockRegistersPdiOffset = isset($pdiInterfacePropertiesByName['lock_registers_offset']->value)
                    ? $this->stringToInt($pdiInterfacePropertiesByName['lock_registers_offset']->value) : null;
            }

            if (isset($this->peripheralModulesByName['nvm'])) {
                $nvmModule = $this->peripheralModulesByName['nvm'];

                if (isset($nvmModule->instancesMappedByName['nvm'])) {
                    $nvmInstance = $nvmModule->instancesMappedByName['nvm'];

                    if (isset($nvmInstance->registerGroupsMappedByName['nvm'])) {
                        $this->nvmModuleBaseAddress = $nvmInstance->registerGroupsMappedByName['nvm']->offset;
                    }
                }
            }

            if (isset($this->peripheralModulesByName['mcu'])) {
                $mcuModule = $this->peripheralModulesByName['mcu'];

                if (isset($mcuModule->instancesMappedByName['mcu'])) {
                    $mcuInstance = $mcuModule->instancesMappedByName['mcu'];

                    if (isset($mcuInstance->registerGroupsMappedByName['mcu'])) {
                        $this->mcuModuleBaseAddress = $mcuInstance->registerGroupsMappedByName['mcu']->offset;
                    }
                }
            }
        }

        if (in_array(Avr8TargetDescriptionFile::AVR8_PHYSICAL_INTERFACE_UPDI, $this->debugPhysicalInterfaces)) {
            if (isset($this->peripheralModulesByName['nvmctrl'])) {
                $nvmModule = $this->peripheralModulesByName['nvmctrl'];

                if (isset($nvmModule->instancesMappedByName['nvmctrl'])) {
                    $nvmInstance = $nvmModule->instancesMappedByName['nvmctrl'];

                    if (isset($nvmInstance->registerGroupsMappedByName['nvmctrl'])) {
                        $this->nvmModuleBaseAddress = $nvmInstance->registerGroupsMappedByName['nvmctrl']->offset;
                    }
                }
            }

            if (isset($this->propertyGroupsByName['updi_interface'])) {
                $updiInterfacePropertyGroup = $this->propertyGroupsByName['updi_interface'];
                $updiInterfacePropertiesByName = $updiInterfacePropertyGroup->propertiesMappedByName;

                if (isset($updiInterfacePropertiesByName['ocd_base_addr'])) {
                    $this->ocdBaseAddress = isset($updiInterfacePropertiesByName['ocd_base_addr']->value)
                        ? $this->stringToInt($updiInterfacePropertiesByName['ocd_base_addr']->value) : null;
                }

                if (isset($updiInterfacePropertiesByName['progmem_offset'])) {
                    $this->programMemoryUpdiStartAddress = isset($updiInterfacePropertiesByName['progmem_offset']->value)
                        ? $this->stringToInt($updiInterfacePropertiesByName['progmem_offset']->value) : null;
                }
            }

            if (!is_null($dataAddressSpace)) {
                if (isset($dataAddressSpace->memorySegmentsByTypeAndName['signatures']['signatures'])) {
                    $signatureMemSegment = $dataAddressSpace->memorySegmentsByTypeAndName['signatures']['signatures'];
                    $this->signatureSegmentSize = isset($signatureMemSegment->size)
                        ? $this->stringToInt($signatureMemSegment->size) : null;

                    $this->signatureSegmentStartAddress = isset($signatureMemSegment->startAddress)
                        ? $this->stringToInt($signatureMemSegment->startAddress) : null;
                }

                if (isset($dataAddressSpace->memorySegmentsByTypeAndName['fuses']['fuses'])) {
                    $fusesMemSegment = $dataAddressSpace->memorySegmentsByTypeAndName['fuses']['fuses'];
                    $this->fuseSegmentSize = isset($fusesMemSegment->size)
                        ? $this->stringToInt($fusesMemSegment->size) : null;

                    $this->fuseSegmentStartAddress = isset($fusesMemSegment->startAddress)
                        ? $this->stringToInt($fusesMemSegment->startAddress) : null;
                }

                if (isset($dataAddressSpace->memorySegmentsByTypeAndName['lockbits']['lockbits'])) {
                    $lockbitsMemSegment = $dataAddressSpace->memorySegmentsByTypeAndName['lockbits']['lockbits'];
                    $this->lockbitsSegmentStartAddress = isset($lockbitsMemSegment->startAddress)
                        ? $this->stringToInt($lockbitsMemSegment->startAddress) : null;
                }
            }
        }
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

        if (is_null($this->signature)
            || is_null($this->signature->byteZero)
            || is_null($this->signature->byteOne)
            || is_null($this->signature->byteTwo)
        ) {
            $failures[] = "Missing or incomplete AVR signature.";
        }

        if (is_null($this->debugPhysicalInterfaces)) {
            $failures[] = 'Target does not support any known AVR8 debug interface - the TDF will need to be deleted.'
                . ' Aborting validation.';
            return $failures;
        }

        if (is_null($this->family) || $this->family == self::AVR8_FAMILY_OTHER) {
            $failures[] = 'Unknown AVR8 family';
        }

        if (in_array(self::AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE, $this->debugPhysicalInterfaces)) {
            if (!isset($this->physicalInterfacesByName['isp'])) {
                $failures[] = 'Missing ISP interface for debugWire target.';
            }

            if (is_null($this->ispProgramModeTimeout)) {
                $failures[] = 'Missing ispenterprogmode_timeout ISP parameter.';
            }
            if (is_null($this->ispProgramModeStabilizationDelay)) {
                $failures[] = 'Missing ispenterprogmode_stabdelay ISP parameter.';
            }
            if (is_null($this->ispProgramModeCommandExecutionDelay)) {
                $failures[] = 'Missing ispenterprogmode_cmdexedelay ISP parameter.';
            }
            if (is_null($this->ispProgramModeSyncLoops)) {
                $failures[] = 'Missing ispenterprogmode_synchloops ISP parameter.';
            }
            if (is_null($this->ispProgramModeByteDelay)) {
                $failures[] = 'Missing ispenterprogmode_bytedelay ISP parameter.';
            }
            if (is_null($this->ispProgramModePollValue)) {
                $failures[] = 'Missing ispenterprogmode_pollvalue ISP parameter.';
            }
            if (is_null($this->ispProgramModePollIndex)) {
                $failures[] = 'Missing ispenterprogmode_pollindex ISP parameter.';
            }
            if (is_null($this->ispProgramModePreDelay)) {
                $failures[] = 'Missing ispleaveprogmode_predelay ISP parameter.';
            }
            if (is_null($this->ispProgramModePostDelay)) {
                $failures[] = 'Missing ispleaveprogmode_postdelay ISP parameter.';
            }
            if (is_null($this->ispReadSignaturePollIndex)) {
                $failures[] = 'Missing ispreadsign_pollindex ISP parameter.';
            }
            if (is_null($this->ispReadFusePollIndex)) {
                $failures[] = 'Missing ispreadfuse_pollindex ISP parameter.';
            }
            if (is_null($this->ispReadLockPollIndex)) {
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

        if (is_null($this->stackPointerRegisterLowAddress)) {
            $failures[] = 'Missing stack pointer register start address.';
        }

        if ($this->stackPointerRegisterSize > 2) {
            // The AVR architecture implementation in GDB expects all SP registers to be a maximum of two bytes in size.
            $failures[] = 'Stack pointer register size (' . $this->stackPointerRegisterSize . ') exceeds maximum (2).';
        }

        if ($this->stackPointerRegisterSize < 1) {
            $failures[] = 'Stack pointer register size (' . $this->stackPointerRegisterSize . ') is less than 1.';
        }

        if (is_null($this->stackPointerRegisterSize)) {
            $failures[] = 'Missing stack pointer register size.';
        }

        if (is_null($this->statusRegisterStartAddress)) {
            $failures[] = 'Missing status register start address.';
        }

        if (is_null($this->statusRegisterSize)) {
            $failures[] = 'Missing status register size.';

        } else if ($this->statusRegisterSize > 1) {
            $failures[] = 'Status register size exceeds 1';
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

        if ($this->eepromSize < 64) {
            /*
             * All AVR8 targets supported by Bloom have at least 64 bytes of EEPROM. This is assumed in some areas of
             * Bloom's code.
             *
             * The purpose of this check is to remind me to address those areas of Bloom's code before adding support
             * for an AVR8 target with no EEPROM.
             */
            $failures[] = 'Unexpected eeprom size.';
        }

        if (is_null($this->eepromPageSize)) {
            $failures[] = 'Missing eeprom page size.';
        }

        if (is_null($this->eepromStartAddress)) {
            $failures[] = 'Missing eeprom start address.';
        }

        if (
            in_array(self::AVR8_PHYSICAL_INTERFACE_DEBUG_WIRE, $this->debugPhysicalInterfaces)
            || (
                in_array(self::AVR8_PHYSICAL_INTERFACE_JTAG, $this->debugPhysicalInterfaces)
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

        if (in_array(self::AVR8_PHYSICAL_INTERFACE_PDI, $this->debugPhysicalInterfaces)) {
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

            if (is_null($this->nvmModuleBaseAddress)) {
                $failures[] = 'Missing NVM module base address.';
            }

            if (is_null($this->mcuModuleBaseAddress)) {
                $failures[] = 'Missing MCU module base address.';
            }

            if (is_null($this->appSectionStartAddress)) {
                $failures[] = 'Missing APP section start address';
            }

            if (is_null($this->appSectionSize)) {
                $failures[] = 'Missing APP section size';
            }
        }

        if (in_array(Avr8TargetDescriptionFile::AVR8_PHYSICAL_INTERFACE_UPDI, $this->debugPhysicalInterfaces)) {
            if (is_null($this->nvmModuleBaseAddress)) {
                $failures[] = 'Missing NVM base address.';
            }

            if (is_null($this->programMemoryUpdiStartAddress)) {
                $failures[] = 'Missing UPDI program memory offset.';

            } else if ($this->programMemoryUpdiStartAddress > 0xFFFFFF) {
                /*
                 * Due to size constraints of EDBG AVR8 parameters for UPDI sessions, the program memory offset must
                 * fit into a 24-bit integer.
                 */
                $failures[] = 'UPDI program memory offset exceeds maximum value for 24-bit unsigned integer.';
            }

            if (!is_null($this->flashPageSize) && $this->flashPageSize > 0xFFFF) {
                $failures[] = 'Flash page size exceeds maximum value for 16-bit unsigned integer.';
            }

            if (is_null($this->ocdBaseAddress)) {
                $failures[] = 'Missing OCD base address.';

            } else if ($this->ocdBaseAddress > 0xFFFF) {
                $failures[] = 'UPDI OCD base address exceeds maximum value for 16-bit unsigned integer.';
            }

            if (is_null($this->signatureSegmentStartAddress)) {
                $failures[] = 'Missing signature segment start address.';
            }

            if (is_null($this->fuseSegmentSize)) {
                $failures[] = 'Missing fuse segment size.';
            }

            if (is_null($this->fuseSegmentStartAddress)) {
                $failures[] = 'Missing fuses segment start address.';
            }

            if (is_null($this->fuseSegmentSize)) {
                $failures[] = 'Missing fuses segment size.';
            }

            if (is_null($this->lockbitsSegmentStartAddress)) {
                $failures[] = 'Missing lockbits segment start address.';
            }
        }

        if (
            in_array(Avr8TargetDescriptionFile::AVR8_PHYSICAL_INTERFACE_JTAG, $this->debugPhysicalInterfaces)
            && $this->family == self::AVR8_FAMILY_MEGA
        ) {
            if (empty($this->getFuseBitsDescriptor('ocden'))) {
                $failures[] = 'Could not find OCDEN fuse bit field for JTAG target.';

            }

            if (empty($this->getFuseBitsDescriptor('jtagen'))) {
                $failures[] = 'Could not find JTAGEN fuse bit field for JTAG target.';

            }
        }

        if (
            in_array(Avr8TargetDescriptionFile::AVR8_PHYSICAL_INTERFACE_JTAG, $this->debugPhysicalInterfaces)
            || in_array(Avr8TargetDescriptionFile::AVR8_PHYSICAL_INTERFACE_UPDI, $this->debugPhysicalInterfaces)
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
