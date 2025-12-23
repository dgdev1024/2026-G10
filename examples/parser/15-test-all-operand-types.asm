; Test file for all operand types side by side
; Located in: examples/parser/15-test-all-operand-types.asm

.global main

main:
    ; Register operands
    ld d0, d1
    add w0, w1
    sub l0, l1
    
    ; Immediate operands
    ld d0, 0x1000
    add w0, 100
    cmp l0, 'Z'
    
    ; Direct memory operands (address expressions)
    ld d0, [0x80000000]
    st [0x80000004], d0
    add d1, [0x80000008]
    
    ; Indirect memory operands (register contains address)
    ld d0, [d1]
    st [d2], d0
    add d0, [d1]
    
    ; Complex combinations
loop:
    ; Load base address into d0
    ld d0, 0x80000000
    
    ; Load value from address in d0 into d1
    ld d1, [d0]
    
    ; Add immediate to d1
    add d1, 0x1
    
    ; Store d1 back to address in d0
    st [d0], d1
    
    ; Load another value from direct address
    ld d2, [0x80000004]
    
    ; Compare with immediate
    cmp d1, d2
    
done:
    halt
