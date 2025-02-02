<?php
namespace Targets\TargetDescriptionFiles\RiscV\Services;

use Targets\TargetDescriptionFiles\TargetDescriptionFile;

require_once __DIR__ . '/../../Services/ValidationService.php';
require_once __DIR__ . '/../../TargetDescriptionFile.php';

class ValidationService extends \Targets\TargetDescriptionFiles\Services\ValidationService
{
    public function validateRiscVTdf(TargetDescriptionFile $tdf): array
    {
        $failures = parent::validateTdf($tdf);

        if ($tdf->getAddressSpace('system') === null) {
            $failures[] = 'Missing system address space';
        }

        if ($tdf->getAddressSpace('csr') === null) {
            $failures[] = 'Missing CSR address space';
        }

        if ($tdf->getAddressSpace('gpr') === null) {
            $failures[] = 'Missing GPR address space';
        }

        $mainProgramSegment = $tdf->getMemorySegment('system', 'main_program');
        if ($mainProgramSegment === null) {
            $failures[] = 'Missing main program memory segment';
        }

        $bootProgramSegment = $tdf->getMemorySegment('system', 'boot_program');
        if ($bootProgramSegment === null) {
            $failures[] = 'Missing boot program memory segment';
        }

        if (
            $mainProgramSegment !== null
            && $bootProgramSegment !== null
            && $mainProgramSegment->size() <= $bootProgramSegment->size()
        ) {
            $failures[] = 'Main program segment size is not greater than the boot segment size';
        }

        if ($tdf->getMemorySegment('system', 'internal_ram') === null) {
            $failures[] = 'Missing internal ram memory segment';
        }

        if ($tdf->getProperty('wch_link_interface', 'programming_opcode_key') === null) {
            $failures[] = 'Missing "programming_opcode_key" property';
        }

        $programmingBlockSize = $tdf->getPropertyValue('wch_link_interface', 'programming_block_size');
        if (!is_numeric($programmingBlockSize)) {
            $failures[] = 'Missing/invalid "programming_block_size" property';

        } else {
            if (
                $mainProgramSegment !== null
                && !$this->alignsWith($mainProgramSegment->addressRange->startAddress, (int)$programmingBlockSize)
            ) {
                $failures[] = 'Main program memory segment start address does not align with programming block size';
            }

            if (
                $bootProgramSegment !== null
                && !$this->alignsWith($bootProgramSegment->addressRange->startAddress, (int)$programmingBlockSize)
            ) {
                $failures[] = 'Boot program memory segment start address does not align with programming block size';
            }
        }

        /*
         * The partial block write alignment size is typically 2 bytes, but due to a bug in WCH-Link firmware 2.9, we
         * sometimes have to use an alignment size of 64 bytes.
         *
         * See WchLinkDebugInterface::writeProgramMemoryPartialBlock() for more.
         */
        $wchLinkPartialWriteAlignmentSize = 64;
        if ($mainProgramSegment !== null) {
            if (
                !$this->alignsWith(
                    $mainProgramSegment->addressRange->startAddress,
                    $wchLinkPartialWriteAlignmentSize
                )
            ) {
                $failures[] = 'Main program memory segment start address does not align with partial write alignment '
                    . 'size';
            }

            if (!$this->alignsWith($mainProgramSegment->addressRange->size(), $wchLinkPartialWriteAlignmentSize)) {
                $failures[] = 'Main program memory segment size does not align with partial write alignment size';
            }

            if ($mainProgramSegment->pageSize === null) {
                $failures[] = 'Missing page size in main program memory segment';

            } elseif (!$this->alignsWith($mainProgramSegment->pageSize, $wchLinkPartialWriteAlignmentSize)) {
                $failures[] = 'Main program memory segment page size does not align with partial write alignment size';
            }
        }

        if ($bootProgramSegment !== null) {
            if (
                !$this->alignsWith(
                    $bootProgramSegment->addressRange->startAddress,
                    $wchLinkPartialWriteAlignmentSize
                )
            ) {
                $failures[] = 'Boot program memory segment start address does not align with partial write alignment '
                    . 'size';
            }

            if (!$this->alignsWith($bootProgramSegment->addressRange->size(), $wchLinkPartialWriteAlignmentSize)) {
                $failures[] = 'Boot program memory segment size does not align with partial write alignment size';
            }

            if ($bootProgramSegment->pageSize === null) {
                $failures[] = 'Missing page size in boot program memory segment';

            } elseif (!$this->alignsWith($bootProgramSegment->pageSize, $wchLinkPartialWriteAlignmentSize)) {
                $failures[] = 'Boot program memory segment page size does not align with partial write alignment size';
            }
        }

        if ($tdf->getProperty('riscv_debug_module', 'trigger_count') === null) {
            $failures[] = 'Missing "trigger_count" property';
        }

        return $failures;
    }
}
