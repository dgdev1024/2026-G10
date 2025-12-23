; Test file for instructions with register operands
; Located in: examples/parser/9-test-instructions-registers.asm

; Instructions with one register operand
not d0
not w1
not l2
inc d3
dec d4

; Instructions with two register operands
ld d0, d1
add w0, w1
sub l0, l1
xor d2, d3
mv w4, w5

; Mixed with labels
start:
    ld d0, d1
    add w0, w1
    not l0
loop:
    dec d0
