; Test file for instructions with immediate operands
; Located in: examples/parser/10-test-instructions-immediates.asm

; Instructions with immediate values (hex)
ld d0, 0x10
add w0, 0x20
sub l0, 0x30

; Instructions with immediate values (decimal)
ld d1, 42
add w1, 100
cmp l1, 255

; Instructions with immediate values (binary)
ld d2, 0b10101010
add w2, 0b11110000

; Instructions with character literals
cmp l0, 'A'
ld l1, 'Z'

; Mixed with labels and registers
start:
    ld d0, 0x1000
    add d0, 0x100
    mv w0, d0
    cmp w0, 0x1100
