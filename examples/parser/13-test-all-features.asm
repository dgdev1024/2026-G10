; Comprehensive parser test demonstrating all implemented features
; Located in: examples/parser/13-test-all-features.asm

; Directives
.global main, helper, data_start
.extern printf

; Labels and data definition would go here (not yet implemented)
data_start:

; Main function
main:
    ; Instructions with no operands
    nop
    di
    
    ; Instructions with register operands
    not d0
    inc w0
    dec l0
    
    ; Instructions with two register operands
    ld d0, d1
    add w0, w1
    sub l0, l1
    mv d2, d3
    
    ; Instructions with immediate operands
    ld d0, 0x1000
    add w0, 100
    cmp l0, 'A'
    sub d1, 0b11111111
    
    ; Instructions with direct memory operands
    ld d0, [0x80000000]
    st [0x80000004], d0
    add d1, [0x80000008]
    
    ; Complex instruction sequences
loop:
    ld d0, [0x80000000]
    add d0, 0x1
    st [0x80000000], d0
    cmp d0, 0x100
    dec w0
    
helper:
    ld l0, 42
    not l0
    
done:
    halt
