; Test 19: Instructions with Expression Operands
; Tests the parser's ability to handle expressions as operands
; in various instruction types.

.org 0x2000

; Load instructions with expression operands
test_load_expressions:
    ld d0, 10 + 20
    ld d0, 0x100 * 4
    ld d0, 1 << 16
    ld d0, 0xFF & 0x0F | 0xF0
    ld w0, 0x1000 + 0x234
    ld l0, 'A' + 1

; Store with direct address expressions
.org 0x80000000
ram_base:

.org 0x2100
test_store_expressions:
    st [0x80000000 + 0x100], d0
    st [ram_base + 4], d0
    st [ram_base + 8 * 2], d0

; Load from direct address expressions
test_load_direct_expressions:
    ld d0, [0x80000000 + 0x100]
    ld d0, [ram_base + 4]
    ld d0, [ram_base + 16 * 4]

; Quick RAM with expressions
test_quick_expressions:
    ldq d0, [0x1000 + 0x100]
    stq [0x100 * 2], d0

; I/O port with expressions
test_io_expressions:
    ldp l0, [0x10 + 0x05]
    stp [0x20 - 0x10], l0

; Jump/branch with label arithmetic
start_label:
    nop
end_label:
test_jump_expressions:
    jmp nc, start_label
    jpb nc, end_label

; Compare with expressions (8-bit only uses L0)
test_compare_expressions:
    cmp l0, 100 + 50
    cmp l0, 0xFF & 0x0F

; Arithmetic instructions with expressions (D0 for 32-bit, W0 for 16-bit, L0 for 8-bit)
test_arithmetic_expressions:
    add d0, 10 + 5
    sub d0, 0x100 - 0x10
    and l0, 0xFF & 0xF0
    or l0, 0x0F | 0xF0
    xor l0, 0xAA ^ 0x55

; Bit operations with simple bit numbers (syntax: BIT bit, register)
; Note: Bit operations work on L registers only
test_bit_expressions:
    bit 3, l0               ; Test bit 3 in l0
    set 3, l0               ; Set bit 3 in l0
    res 4, l0               ; Reset bit 4 in l0
