; Test 01: NOP Instruction Encoding
; Tests that NOP instructions are correctly encoded as 0x0000.
; Expected: Each NOP should produce opcode 0x00 0x00

.org 0x2000

test_nop_single:
    nop                         ; Single NOP: 0x0000

test_nop_multiple:
    nop                         ; First NOP
    nop                         ; Second NOP
    nop                         ; Third NOP
    nop                         ; Fourth NOP

test_nop_sequence:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
