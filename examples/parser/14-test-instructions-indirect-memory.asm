; Test file for instructions with indirect memory operands
; Located in: examples/parser/14-test-instructions-indirect-memory.asm

; Instructions with indirect memory operands (register as address)
ld d0, [d1]
st [d2], d0
ld w0, [w1]
st [w2], w0
ld l0, [l1]
st [l2], l0

; Mixed operand types - register and indirect
add d0, [d1]
sub w0, [w1]
xor l0, [l1]

; Mixed operand types - indirect and immediate
ld d0, [d1]
add d0, 0x10
st [d1], d0

; Complex combinations
start:
    ld d0, 0x80000000
    ld d1, [d0]
    add d1, 0x1
    st [d0], d1
    inc d0
    ld d2, [d0]
    cmp d1, d2
