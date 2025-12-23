; Test file for comprehensive instruction tests
; Located in: examples/parser/12-test-instructions-comprehensive.asm

; Labels and directives
.global main
.extern helper

main:
    ; No operands
    nop
    
    ; One register operand
    not d0
    inc w0
    dec l0
    
    ; Two register operands
    ld d0, d1
    add w0, w1
    sub l0, l1
    mv d2, d3
    
    ; Register and immediate
    ld d0, 0x10
    add w0, 42
    cmp l0, 'A'
    
    ; Register and direct memory
    ld d0, [0x80000000]
    st [0x80000004], d0
    add d0, [0x80000008]
    
    ; Complex combinations
loop:
    ld d0, [0x80000000]
    add d0, 0x1
    st [0x80000000], d0
    cmp d0, 0x100
    dec w0
    
helper:
    nop
