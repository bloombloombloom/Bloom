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

        if ($tdf->getProperty('wch_link_interface', 'programming_block_size') === null) {
            $failures[] = 'Missing "programming_block_size" property';
        }

        if ($tdf->getProperty('riscv_debug_module', 'trigger_count') === null) {
            $failures[] = 'Missing "trigger_count" property';
        }

        return $failures;
    }
}
