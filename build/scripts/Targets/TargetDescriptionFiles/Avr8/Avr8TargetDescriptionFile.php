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
require_once __DIR__ . "/AvrIsa.php";
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
            'avr-family' => $this->getAvrFamily()->value ?? '',
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

    public function getAvrIsa(): ?AvrIsa
    {
        return AvrIsa::tryFrom($this->deviceAttributesByName['architecture']);
    }

    public function getDebugWireParameters(): DebugWireParameters
    {
        $output = new DebugWireParameters();

        $programMemorySegment = $this->getProgramMemorySegment();
        if ($programMemorySegment instanceof MemorySegment) {
            $output->flashStartAddress = $programMemorySegment->addressRange->startAddress;
            $output->flashSize = $programMemorySegment->size();
            $output->flashPageSize = $programMemorySegment->pageSize;
        }

        $ramMemorySegment = $this->getRamSegment();
        if ($ramMemorySegment instanceof MemorySegment) {
            $output->ramStartAddress = $ramMemorySegment->addressRange->startAddress;
        }

        $output->bootSectionOptions = $this->getBootSectionOptions();

        $eepromMemorySegment = $this->getEepromSegment();
        if ($eepromMemorySegment instanceof MemorySegment) {
            $output->eepromSize = $eepromMemorySegment->size();
            $output->eepromPageSize = $eepromMemorySegment->pageSize;
        }

        $output->ocdRevision = $this->stringService->tryStringToInt(
            $this->getPropertyValue('ocd', 'ocd_revision')
        );
        $output->ocdDataRegisterAddress = $this->stringService->tryStringToInt(
            $this->getPropertyValue('ocd', 'ocd_datareg')
        );

        $eepromPeripheral = $this->getTargetPeripheral('eeprom');

        if ($eepromPeripheral instanceof TargetPeripheral) {
            $eepromAddressRegister = $eepromPeripheral->getRegister('eeprom', 'eear');

            if ($eepromAddressRegister instanceof TargetRegister) {
                $output->eearAddressLow = $eepromAddressRegister->address;
                $output->eearAddressHigh = $eepromAddressRegister->address + ($eepromAddressRegister->size - 1);

            } else {
                $eearAddressLow = $eepromPeripheral->getRegister('eeprom', 'eearl');
                $eearAddressHigh = $eepromPeripheral->getRegister('eeprom', 'eearh');

                if ($eearAddressLow instanceof TargetRegister) {
                    $output->eearAddressLow = $eearAddressLow->address;
                    $output->eearAddressHigh = $eearAddressLow->address + ($eearAddressLow->size - 1);
                }

                if ($eearAddressHigh instanceof TargetRegister) {
                    $output->eearAddressHigh = $eearAddressHigh->address;
                }
            }

            $eepromDataRegister = $eepromPeripheral->getRegister('eeprom', 'eedr');
            if ($eepromDataRegister instanceof TargetRegister) {
                $output->eedrAddress = $eepromDataRegister->address;
            }

            $eepromControlRegister = $eepromPeripheral->getRegister('eeprom', 'eecr');
            if ($eepromControlRegister instanceof TargetRegister) {
                $output->eecrAddress = $eepromControlRegister->address;
            }
        }

        $cpuPeripheral = $this->getTargetPeripheral('cpu');

        if ($cpuPeripheral instanceof TargetPeripheral) {
            $spmcsRegister = $cpuPeripheral->getRegister('cpu', 'spmcsr')
                ?? $cpuPeripheral->getRegister('cpu', 'spmcr');

            if ($spmcsRegister instanceof TargetRegister) {
                $output->spmcrAddress = $spmcsRegister->address;
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

        if ($output->spmcrAddress === null) {
            $bootLoaderPeripheral = $this->getTargetPeripheral('boot_load');

            if ($bootLoaderPeripheral instanceof TargetPeripheral) {
                $spmcsRegister = $bootLoaderPeripheral->getRegister('boot_load', 'spmcr')
                    ?? $bootLoaderPeripheral->getRegister('boot_load', 'spmcsr');

                if ($spmcsRegister instanceof TargetRegister) {
                    $output->spmcrAddress = $spmcsRegister->address;
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
            $output->flashStartAddress = $programMemorySegment->addressRange->startAddress;
            $output->flashSize = $programMemorySegment->size();
            $output->flashPageSize = $programMemorySegment->pageSize;
        }

        $ramMemorySegment = $this->getRamSegment();
        if ($ramMemorySegment instanceof MemorySegment) {
            $output->ramStartAddress = $ramMemorySegment->addressRange->startAddress;
        }

        $output->bootSectionOptions = $this->getBootSectionOptions();

        $eepromMemorySegment = $this->getEepromSegment();
        if ($eepromMemorySegment instanceof MemorySegment) {
            $output->eepromSize = $eepromMemorySegment->size();
            $output->eepromPageSize = $eepromMemorySegment->pageSize;
        }

        $output->ocdRevision = $this->stringService->tryStringToInt($this->getPropertyValue('ocd', 'ocd_revision'));
        $output->ocdDataRegisterAddress = $this->stringService->tryStringToInt($this->getPropertyValue('ocd', 'ocd_datareg'));

        $eepromPeripheral = $this->getTargetPeripheral('eeprom');

        if ($eepromPeripheral instanceof TargetPeripheral) {
            $eepromAddressRegister = $eepromPeripheral->getRegister('eeprom', 'eear');

            if ($eepromAddressRegister instanceof TargetRegister) {
                $output->eearAddressLow = $eepromAddressRegister->address;
                $output->eearAddressHigh = $eepromAddressRegister->address + ($eepromAddressRegister->size - 1);

            } else {
                $eearAddressLow = $eepromPeripheral->getRegister('eeprom', 'eearl');
                $eearAddressHigh = $eepromPeripheral->getRegister('eeprom', 'eearh');

                if ($eearAddressLow instanceof TargetRegister) {
                    $output->eearAddressLow = $eearAddressLow->address;
                }

                if ($eearAddressHigh instanceof TargetRegister) {
                    $output->eearAddressHigh = $eearAddressHigh->address;
                }
            }

            $eepromDataRegister = $eepromPeripheral->getRegister('eeprom', 'eedr');
            if ($eepromDataRegister instanceof TargetRegister) {
                $output->eedrAddress = $eepromDataRegister->address;
            }

            $eepromControlRegister = $eepromPeripheral->getRegister('eeprom', 'eecr');
            if ($eepromControlRegister instanceof TargetRegister) {
                $output->eecrAddress = $eepromControlRegister->address;
            }
        }

        $cpuPeripheral = $this->getTargetPeripheral('cpu');

        if ($cpuPeripheral instanceof TargetPeripheral) {
            $spmcsRegister = $cpuPeripheral->getRegister('cpu', 'spmcsr')
                ?? $cpuPeripheral->getRegister('cpu', 'spmcr');

            if ($spmcsRegister instanceof TargetRegister) {
                $output->spmcrAddress = $spmcsRegister->address;
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

        if ($output->spmcrAddress === null) {
            $bootLoaderPeripheral = $this->getTargetPeripheral('boot_load');

            if ($bootLoaderPeripheral instanceof TargetPeripheral) {
                $spmcsRegister = $bootLoaderPeripheral->getRegister('boot_load', 'spmcr')
                    ?? $bootLoaderPeripheral->getRegister('boot_load', 'spmcsr');

                if ($spmcsRegister instanceof TargetRegister) {
                    $output->spmcrAddress = $spmcsRegister->address;
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
        $output->productionSignaturesOffset = $this->stringService->tryStringToInt(
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
                $output->appSectionSize = $appSection->size();
            }

            $bootSection = $programMemorySegment->getSection('boot_section');
            if ($bootSection instanceof MemorySegmentSection) {
                $output->bootSectionSize = $bootSection->size();
            }
        }

        $eepromMemorySegment = $this->getEepromSegment();
        if ($eepromMemorySegment instanceof MemorySegment) {
            $output->eepromSize = $eepromMemorySegment->size();
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
            $output->flashSize = $programMemorySegment->size();
            $output->flashPageSize = $programMemorySegment->pageSize;
        }

        $eepromMemorySegment = $this->getEepromSegment();
        if ($eepromMemorySegment instanceof MemorySegment) {
            $output->eepromStartAddress = $eepromMemorySegment->addressRange->startAddress;
            $output->eepromSize = $eepromMemorySegment->size();
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
            'signatures',
            'signatures'
        ) ?? $this->getMemorySegment(
            'data',
            'signatures'
        );

        if ($signatureMemorySegment instanceof MemorySegment) {
            $output->signatureSegmentStartAddress = $signatureMemorySegment->addressRange->startAddress;
        }

        $fuseMemorySegment = $this->getMemorySegment(
            'fuses',
            'fuses'
        ) ?? $this->getMemorySegment(
            'data',
            'fuses'
        );

        if ($fuseMemorySegment instanceof MemorySegment) {
            $output->fuseSegmentSize = $fuseMemorySegment->size();
            $output->fuseSegmentStartAddress = $fuseMemorySegment->addressRange->startAddress;
        }

        $lockbitsMemorySegment = $this->getMemorySegment(
            'lockbits',
            'lockbits'
        ) ?? $this->getMemorySegment(
            'data',
            'lockbits'
        );

        if ($lockbitsMemorySegment instanceof MemorySegment) {
            $output->lockbitsSegmentStartAddress = $lockbitsMemorySegment->addressRange->startAddress;
        }

        return $output;
    }

    public function getFuseTargetPeripheral(): ?TargetPeripheral
    {
        return $this->getTargetPeripheral('fuse');
    }

    public function getFuseTargetRegisterGroup(): ?TargetRegisterGroup
    {
        return $this->getFuseTargetPeripheral()?->getRegisterGroup("fuse");
    }

    public function getFuseBitsDescriptor(string $fuseBitFieldKey): ?FuseBitsDescriptor
    {
        $fuseRegisterGroup = $this->getFuseTargetRegisterGroup();
        if ($fuseRegisterGroup instanceof TargetRegisterGroup) {
            foreach ($fuseRegisterGroup->registers as $fuseRegister) {
                foreach ($fuseRegister->bitFields as $bitField) {
                    if ($bitField->key === $fuseBitFieldKey) {
                        return new FuseBitsDescriptor($fuseRegister->name);
                    }
                }
            }
        }

        return null;
    }

    public function getProgramMemorySegment(): ?MemorySegment
    {
        return $this->getMemorySegment('prog', 'internal_program_memory');
    }

    public function getGpRegistersMemorySegment(): ?MemorySegment
    {
        return $this->getMemorySegment('data', 'gp_registers')
            ?? $this->getMemorySegment('register_file', 'gp_registers');
    }

    public function getRamSegment(): ?MemorySegment
    {
        return $this->getMemorySegment('data', 'internal_ram');
    }

    public function getEepromSegment(): ?MemorySegment
    {
        return $this->getMemorySegment('eeprom', 'internal_eeprom')
            ?? $this->getMemorySegment('data', 'internal_eeprom');
    }

    public function getIoMemorySegment(): ?MemorySegment
    {
        return $this->getMemorySegment('data', 'io')
            ?? $this->getMemorySegment('data', 'mapped_io');
    }

    public function getSignaturesMemorySegment(): ?MemorySegment
    {
        return $this->getMemorySegment('data', 'signatures')
            ?? $this->getMemorySegment('signatures', 'signatures');
    }

    public function getFusesMemorySegment(): ?MemorySegment
    {
        return $this->getMemorySegment('data', 'fuses')
            ?? $this->getMemorySegment('fuses', 'fuses');
    }

    private function getBootSectionOptions(): array
    {
        $output = [];

        $bootSectionPropertyGroups = $this->getPropertyGroup('boot_section_options')->subPropertyGroups ?? [];
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
