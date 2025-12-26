; Test 02: Control Instruction Encoding
; Tests encoding of CPU control instructions.
; Verifies correct opcodes for control flow instructions.

.org 0x2000

test_control_instructions:
    nop                         ; 0x0000 - No operation
    stop                        ; 0x0100 - Stop execution
    halt                        ; 0x0200 - Halt CPU
    di                          ; 0x0300 - Disable interrupts
    ei                          ; 0x0400 - Enable interrupts (after next instr)
    eii                         ; 0x0500 - Enable interrupts immediately

test_accumulator_ops:
    daa                         ; 0x0600 - Decimal adjust accumulator
    scf                         ; 0x0700 - Set carry flag
    ccf                         ; 0x0800 - Complement carry flag
    clv                         ; 0x0900 - Clear overflow flag
    sev                         ; 0x0A00 - Set overflow flag
    sev                         ; 0x0A00 - Set overflow flag
