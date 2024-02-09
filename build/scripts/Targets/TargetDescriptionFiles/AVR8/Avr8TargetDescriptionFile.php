<?php
namespace Targets\TargetDescriptionFiles\Avr8;

use Targets\TargetDescriptionFiles\Services\StringService;
use Targets\TargetDescriptionFiles\TargetDescriptionFile;
use Targets\TargetDescriptionFiles\MemorySegment;
use Targets\TargetDescriptionFiles\MemorySegmentSection;
use Targets\TargetDescriptionFiles\PhysicalInterface;
use Targets\TargetPeripheral;
use Targets\TargetRegister;
use Targets\TargetRegisterGroup;

require_once __DIR__ . "/../TargetDescriptionFile.php";
require_once __DIR__ . "/../Services/StringService.php";
require_once __DIR__ . "/AvrFamily.php";
require_once __DIR__ . "/AvrPhysicalInterface.php";
require_once __DIR__ . "/Signature.php";
require_once __DIR__ . "/DebugWireParameters.php";
require_once __DIR__ . "/IspParameters.php";
require_once __DIR__ . "/JtagParameters.php";
require_once __DIR__ . "/PdiParameters.php";
require_once __DIR__ . "/UpdiParameters.php";
require_once __DIR__ . "/FuseBitsDescriptor.php";

class Avr8TargetDescriptionFile extends TargetDescriptionFile
{
    private StringService $stringService;

    public function __construct(?StringService $stringService = null)
    {
        $this->stringService = $stringService ?? new StringService();
    }

    public function getAdditionalDeviceAttributes(): array
    {
        return [
            'avr-family' => $this->getAvrFamily()->value,
        ];
    }

    public function getSignature(): ?Signature
    {
        $byteZero = $this->getPropertyValue('signatures', 'signature0');
        $byteOne = $this->getPropertyValue('signatures', 'signature1');
        $byteTwo = $this->getPropertyValue('signatures', 'signature2');

        if (!empty($byteZero) && !empty($byteOne) && !empty($byteTwo)) {
            $signature = new Signature();
            $signature->byteZero = $this->stringService->tryStringToInt($byteZero);
            $signature->byteOne = $this->stringService->tryStringToInt($byteOne);
            $signature->byteTwo = $this->stringService->tryStringToInt($byteTwo);

            return $signature;
        }

        return null;
    }

    public function getAvrFamily(): ?AvrFamily
    {
        return AvrFamily::tryFrom($this->deviceAttributesByName['avr-family']);
    }

    public function getSupportedPhysicalInterfaces(): array
    {
        $physicalInterfacesByName = [
            'isp' => AvrPhysicalInterface::ISP,
            'debugwire' => AvrPhysicalInterface::DEBUG_WIRE,
            'updi' => AvrPhysicalInterface::UPDI,
            'pdi' => AvrPhysicalInterface::PDI,
            'jtag' => AvrPhysicalInterface::JTAG,
        ];

        return array_filter(array_map(
            fn (PhysicalInterface $interface): ?AvrPhysicalInterface
                => $physicalInterfacesByName[strtolower($interface->name ?? '')] ?? null,
            $this->physicalInterfaces
        ));
    }

    public function getSupportedDebugPhysicalInterfaces(): array
    {
        $physicalInterfacesByName = [
            'debugwire' => AvrPhysicalInterface::DEBUG_WIRE,
            'updi' => AvrPhysicalInterface::UPDI,
            'pdi' => AvrPhysicalInterface::PDI,
            'jtag' => AvrPhysicalInterface::JTAG,
        ];

        return array_filter(array_map(
            fn (PhysicalInterface $interface): ?AvrPhysicalInterface
                => $physicalInterfacesByName[strtolower($interface->name ?? '')] ?? null,
            $this->physicalInterfaces
        ));
    }

    public function getDebugWireParameters(): DebugWireParameters
    {
        $output = new DebugWireParameters();

        $programMemorySegment = $this->getProgramMemorySegment();
        if ($programMemorySegment instanceof MemorySegment) {
            $output->flashStartAddress = $programMemorySegment->startAddress;
            $output->flashSize = $programMemorySegment->size;
            $output->flashPageSize = $programMemorySegment->pageSize;
        }

        $ramMemorySegment = $this->getRamSegment();
        if ($ramMemorySegment instanceof MemorySegment) {
            $output->ramStartAddress = $ramMemorySegment->startAddress;
        }

        $output->bootSections = $this->getBootSections();

        $eepromMemorySegment = $this->getEepromSegment();
        if ($eepromMemorySegment instanceof MemorySegment) {
            $output->eepromSize = $eepromMemorySegment->size;
            $output->eepromPageSize = $eepromMemorySegment->pageSize;
        }

        $output->ocdRevision = $this->stringService->tryStringToInt($this->getPropertyValue('ocd', 'ocd_revision'));
        $output->ocdDataRegister = $this->stringService->tryStringToInt($this->getPropertyValue('ocd', 'ocd_datareg'));

        $eepromPeripheral = $this->getTargetPeripheral('eeprom');

        if ($eepromPeripheral instanceof TargetPeripheral) {
            $eepromAddressRegister = $eepromPeripheral->getRegister('eeprom', 'eear');

            if ($eepromAddressRegister instanceof TargetRegister) {
                $output->eepromAddressRegisterLow = $eepromAddressRegister->address;
                $output->eepromAddressRegisterHigh = ($eepromAddressRegister->size == 2)
                    ? $eepromAddressRegister->address + 1
                    : $eepromAddressRegister->address;

            } else {
                $eepromAddressRegisterLow = $eepromPeripheral->getRegister('eeprom', 'eearl');
                $eepromAddressRegisterHigh = $eepromPeripheral->getRegister('eeprom', 'eearh');

                if ($eepromAddressRegisterLow instanceof TargetRegister) {
                    $output->eepromAddressRegisterLow = $eepromAddressRegisterLow->address;
                    $output->eepromAddressRegisterHigh = $eepromAddressRegisterLow->address;
                }

                if ($eepromAddressRegisterHigh instanceof TargetRegister) {
                    $output->eepromAddressRegisterHigh = $eepromAddressRegisterHigh->address;
                }
            }

            $eepromDataRegister = $eepromPeripheral->getRegister('eeprom', 'eedr');
            if ($eepromDataRegister instanceof TargetRegister) {
                $output->eepromDataRegisterAddress = $eepromDataRegister->address;
            }

            $eepromControlRegister = $eepromPeripheral->getRegister('eeprom', 'eecr');
            if ($eepromControlRegister instanceof TargetRegister) {
                $output->eepromControlRegisterAddress = $eepromControlRegister->address;
            }
        }

        $cpuPeripheral = $this->getTargetPeripheral('cpu');

        if ($cpuPeripheral instanceof TargetPeripheral) {
            $spmcsRegister = $cpuPeripheral->getRegister('cpu', 'spmcsr')
                ?? $cpuPeripheral->getRegister('cpu', 'spmcr');

            if ($spmcsRegister instanceof TargetRegister) {
                $output->spmcRegisterStartAddress = $spmcsRegister->address;
            }

            $osccalRegister = $cpuPeripheral->getRegister('cpu', 'osccal')
                ?? $cpuPeripheral->getRegister('cpu', 'osccal0')
                ?? $cpuPeripheral->getRegister('cpu', 'osccal1')
                ?? $cpuPeripheral->getRegister('cpu', 'fosccal')
                ?? $cpuPeripheral->getRegister('cpu', 'sosccala');

            if ($osccalRegister instanceof TargetRegister) {
                $output->osccalAddress = $osccalRegister->address;
            }
        }

        if ($output->spmcRegisterStartAddress === null) {
            $bootLoaderPeripheral = $this->getTargetPeripheral('boot_load');

            if ($bootLoaderPeripheral instanceof TargetPeripheral) {
                $spmcsRegister = $bootLoaderPeripheral->getRegister('boot_load', 'spmcr')
                    ?? $bootLoaderPeripheral->getRegister('boot_load', 'spmcsr');

                if ($spmcsRegister instanceof TargetRegister) {
                    $output->spmcRegisterStartAddress = $spmcsRegister->address;
                }
            }
        }

        return $output;
    }

    public function getIspParameters(): IspParameters
    {
        $output = new IspParameters();

        $output->programModeTimeout = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_timeout')
        );
        $output->programModeStabilizationDelay = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_stabdelay')
        );
        $output->programModeCommandExecutionDelay = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_cmdexedelay')
        );
        $output->programModeSyncLoops = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_synchloops')
        );
        $output->programModeByteDelay = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_bytedelay')
        );
        $output->programModePollValue = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_pollvalue')
        );
        $output->programModePollIndex = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispenterprogmode_pollindex')
        );
        $output->programModePreDelay = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispleaveprogmode_predelay')
        );
        $output->programModePostDelay = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispleaveprogmode_postdelay')
        );
        $output->readSignaturePollIndex = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispreadsign_pollindex')
        );
        $output->readFusePollIndex = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispreadfuse_pollindex')
        );
        $output->readLockPollIndex = $this->stringService->tryStringToInt(
            $this->getPropertyValue('isp_interface', 'ispreadlock_pollindex')
        );

        return $output;
    }

    public function getJtagParameters(): JtagParameters
    {
        $output = new JtagParameters();

        $programMemorySegment = $this->getProgramMemorySegment();
        if ($programMemorySegment instanceof MemorySegment) {
            $output->flashStartAddress = $programMemorySegment->startAddress;
            $output->flashSize = $programMemorySegment->size;
            $output->flashPageSize = $programMemorySegment->pageSize;
        }

        $ramMemorySegment = $this->getRamSegment();
        if ($ramMemorySegment instanceof MemorySegment) {
            $output->ramStartAddress = $ramMemorySegment->startAddress;
        }

        $output->bootSections = $this->getBootSections();

        $eepromMemorySegment = $this->getEepromSegment();
        if ($eepromMemorySegment instanceof MemorySegment) {
            $output->eepromSize = $eepromMemorySegment->size;
            $output->eepromPageSize = $eepromMemorySegment->pageSize;
        }

        $output->ocdRevision = $this->stringService->tryStringToInt($this->getPropertyValue('ocd', 'ocd_revision'));
        $output->ocdDataRegister = $this->stringService->tryStringToInt($this->getPropertyValue('ocd', 'ocd_datareg'));

        $eepromPeripheral = $this->getTargetPeripheral('eeprom');

        if ($eepromPeripheral instanceof TargetPeripheral) {
            $eepromAddressRegister = $eepromPeripheral->getRegister('eeprom', 'eear');

            if ($eepromAddressRegister instanceof TargetRegister) {
                $output->eepromAddressRegisterLow = $eepromAddressRegister->address;
                $output->eepromAddressRegisterHigh = ($eepromAddressRegister->size == 2)
                    ? $eepromAddressRegister->address + 1
                    : $eepromAddressRegister->address;

            } else {
                $eepromAddressRegisterLow = $eepromPeripheral->getRegister('eeprom', 'eearl');
                $eepromAddressRegisterHigh = $eepromPeripheral->getRegister('eeprom', 'eearh');

                if ($eepromAddressRegisterLow instanceof TargetRegister) {
                    $output->eepromAddressRegisterLow = $eepromAddressRegisterLow->address;
                    $output->eepromAddressRegisterHigh = $eepromAddressRegisterLow->address;
                }

                if ($eepromAddressRegisterHigh instanceof TargetRegister) {
                    $output->eepromAddressRegisterHigh = $eepromAddressRegisterHigh->address;
                }
            }

            $eepromDataRegister = $eepromPeripheral->getRegister('eeprom', 'eedr');
            if ($eepromDataRegister instanceof TargetRegister) {
                $output->eepromDataRegisterAddress = $eepromDataRegister->address;
            }

            $eepromControlRegister = $eepromPeripheral->getRegister('eeprom', 'eecr');
            if ($eepromControlRegister instanceof TargetRegister) {
                $output->eepromControlRegisterAddress = $eepromControlRegister->address;
            }
        }

        $cpuPeripheral = $this->getTargetPeripheral('cpu');

        if ($cpuPeripheral instanceof TargetPeripheral) {
            $spmcsRegister = $cpuPeripheral->getRegister('cpu', 'spmcsr')
                ?? $cpuPeripheral->getRegister('cpu', 'spmcr');

            if ($spmcsRegister instanceof TargetRegister) {
                $output->spmcRegisterStartAddress = $spmcsRegister->address;
            }

            $osccalRegister = $cpuPeripheral->getRegister('cpu', 'osccal')
                ?? $cpuPeripheral->getRegister('cpu', 'osccal0')
                ?? $cpuPeripheral->getRegister('cpu', 'osccal1')
                ?? $cpuPeripheral->getRegister('cpu', 'fosccal')
                ?? $cpuPeripheral->getRegister('cpu', 'sosccala');

            if ($osccalRegister instanceof TargetRegister) {
                $output->osccalAddress = $osccalRegister->address;
            }
        }

        if ($output->spmcRegisterStartAddress === null) {
            $bootLoaderPeripheral = $this->getTargetPeripheral('boot_load');

            if ($bootLoaderPeripheral instanceof TargetPeripheral) {
                $spmcsRegister = $bootLoaderPeripheral->getRegister('boot_load', 'spmcr')
                    ?? $bootLoaderPeripheral->getRegister('boot_load', 'spmcsr');

                if ($spmcsRegister instanceof TargetRegister) {
                    $output->spmcRegisterStartAddress = $spmcsRegister->address;
                }
            }
        }

        return $output;
    }

    public function getPdiParameters(): PdiParameters
    {
        $output = new PdiParameters();

        $output->appSectionOffset = $this->stringService->tryStringToInt(
            $this->getPropertyValue('pdi_interface', 'app_section_offset')
        );
        $output->bootSectionOffset = $this->stringService->tryStringToInt(
            $this->getPropertyValue('pdi_interface', 'boot_section_offset')
        );
        $output->ramOffset = $this->stringService->tryStringToInt(
            $this->getPropertyValue('pdi_interface', 'datamem_offset')
        );
        $output->eepromOffset = $this->stringService->tryStringToInt(
            $this->getPropertyValue('pdi_interface', 'eeprom_offset')
        );
        $output->userSignaturesOffset = $this->stringService->tryStringToInt(
            $this->getPropertyValue('pdi_interface', 'user_signatures_offset')
        );
        $output->productSignaturesOffset = $this->stringService->tryStringToInt(
            $this->getPropertyValue('pdi_interface', 'prod_signatures_offset')
        );
        $output->fuseRegistersOffset = $this->stringService->tryStringToInt(
            $this->getPropertyValue('pdi_interface', 'fuse_registers_offset')
        );
        $output->lockRegistersOffset = $this->stringService->tryStringToInt(
            $this->getPropertyValue('pdi_interface', 'lock_registers_offset')
        );

        $programMemorySegment = $this->getProgramMemorySegment();
        if ($programMemorySegment instanceof MemorySegment) {
            $output->flashPageSize = $programMemorySegment->pageSize;

            $appSection = $programMemorySegment->getSection('app_section');
            if ($appSection instanceof MemorySegmentSection) {
                $output->appSectionSize = $appSection->size;
            }

            $bootSection = $programMemorySegment->getSection('boot_section');
            if ($bootSection instanceof MemorySegmentSection) {
                $output->bootSectionSize = $bootSection->size;
            }
        }

        $eepromMemorySegment = $this->getEepromSegment();
        if ($eepromMemorySegment instanceof MemorySegment) {
            $output->eepromSize = $eepromMemorySegment->size;
            $output->eepromPageSize = $eepromMemorySegment->pageSize;
        }

        $nvmModuleRegisterGroup = $this->getTargetRegisterGroup('nvm', 'nvm');
        if ($nvmModuleRegisterGroup instanceof TargetRegisterGroup) {
            $output->nvmModuleBaseAddress = $nvmModuleRegisterGroup->baseAddress;
        }

        $output->signatureOffset = $this->stringService->tryStringToInt(
            $this->getPropertyValue('pdi_interface', 'signature_offset')
        );

        return $output;
    }

    public function getUpdiParameters(): UpdiParameters
    {
        $output = new UpdiParameters();

        $output->programMemoryOffset = $this->stringService->tryStringToInt(
            $this->getPropertyValue('updi_interface', 'progmem_offset')
        );

        $programMemorySegment = $this->getProgramMemorySegment();
        if ($programMemorySegment instanceof MemorySegment) {
            $output->flashSize = $programMemorySegment->size;
            $output->flashPageSize = $programMemorySegment->pageSize;
        }

        $eepromMemorySegment = $this->getEepromSegment();
        if ($eepromMemorySegment instanceof MemorySegment) {
            $output->eepromStartAddress = $eepromMemorySegment->startAddress;
            $output->eepromSize = $eepromMemorySegment->size;
            $output->eepromPageSize = $eepromMemorySegment->pageSize;
        }

        $output->nvmModuleBaseAddress = $this->getTargetRegisterGroup(
            'nvmctrl',
            'nvmctrl'
        )->baseAddress ?? null;

        $output->ocdBaseAddress = $this->stringService->tryStringToInt(
            $this->getPropertyValue('updi_interface', 'ocd_base_addr')
        );

        $signatureMemorySegment = $this->getMemorySegment(
            'data',
            'signatures'
        );
        if ($signatureMemorySegment instanceof MemorySegment) {
            $output->signatureSegmentStartAddress = $signatureMemorySegment->startAddress;
        }

        $fuseMemorySegment = $this->getMemorySegment(
            'data',
            'fuses'
        );
        if ($fuseMemorySegment instanceof MemorySegment) {
            $output->fuseSegmentSize = $fuseMemorySegment->size;
            $output->fuseSegmentStartAddress = $fuseMemorySegment->startAddress;
        }

        $lockbitsMemorySegment = $this->getMemorySegment(
            'data',
            'lockbits'
        );
        if ($lockbitsMemorySegment instanceof MemorySegment) {
            $output->lockbitsSegmentStartAddress = $lockbitsMemorySegment->startAddress;
        }

        return $output;
    }

    public function getFuseBitsDescriptor(string $fuseBitFieldKey): ?FuseBitsDescriptor
    {
        $peripheral = $this->getTargetPeripheral('fuse') ?? $this->getTargetPeripheral('nvm');
        if ($peripheral instanceof TargetPeripheral) {
            $fuseRegisterGroup = $peripheral->getRegisterGroup('fuse')
                ?? $peripheral->getRegisterGroup('nvm_fuses');

            if ($fuseRegisterGroup instanceof TargetRegisterGroup) {
                foreach ($fuseRegisterGroup->registers as $fuseRegister) {
                    foreach ($fuseRegister->bitFields as $bitField) {
                        if ($bitField->key === $fuseBitFieldKey) {
                            return new FuseBitsDescriptor($fuseRegister->name);
                        }
                    }
                }
            }
        }

        return null;
    }

    private function getProgramMemorySegment(): ?MemorySegment
    {
        return $this->getMemorySegment('prog', 'internal_program_memory');
    }

    private function getRamSegment(): ?MemorySegment
    {
        return $this->getMemorySegment('data', 'internal_ram');
    }

    private function getEepromSegment(): ?MemorySegment
    {
        return $this->getMemorySegment('data', 'internal_eeprom')
            ?? $this->getMemorySegment('eeprom', 'internal_eeprom');
    }

    private function getBootSections(): array
    {
        $output = [];

        $bootSectionPropertyGroups = $this->getPropertyGroup('boot_sections')->subPropertyGroups ?? [];
        foreach ($bootSectionPropertyGroups as $propertyGroup) {
            $output[] = new BootSection(
                $this->stringService->tryStringToInt($propertyGroup->getPropertyValue('start_address')),
                $this->stringService->tryStringToInt($propertyGroup->getPropertyValue('size')),
                $this->stringService->tryStringToInt($propertyGroup->getPropertyValue('page_size'))
            );
        }

        return $output;
    }
}
