; Test file for instructions with direct memory operands
; Located in: examples/parser/11-test-instructions-direct-memory.asm

; Instructions with direct memory addresses (absolute)
ld d0, [0x80000000]
st [0x80000004], d0
ld w0, [0x4000]
st [0x4002], w0

; Instructions with direct memory addresses (expressions)
ld d1, [0x80000000]
st [0x80000008], d1

; Mixed operand types
start:
    ld d0, [0x80000000]
    add d0, 0x10
    st [0x80000004], d0
    ld w0, [0x4000]
    inc w0
    st [0x4002], w0
