; Final comprehensive parser test demonstrating all implemented features
; Including indirect memory operands
; Located in: examples/parser/16-test-complete-parser.asm

; Directives
.global main, helper, buffer_ptr
.extern malloc, free

; Labels
buffer_ptr:

; Main program
main:
    ; ===== No Operand Instructions =====
    nop
    di
    ei
    
    ; ===== Register Operands =====
    ; One register
    not d0
    inc w0
    dec l0
    
    ; Two registers
    ld d0, d1
    add w0, w1
    sub l0, l1
    mv d2, d3
    xor d4, d5
    
    ; ===== Immediate Operands =====
    ld d0, 0x1000       ; Hex literal
    add w0, 100         ; Decimal literal
    cmp l0, 'A'         ; Character literal
    sub d1, 0b11111111  ; Binary literal
    
    ; ===== Direct Memory Operands =====
    ld d0, [0x80000000]     ; Absolute address
    st [0x80000004], d0     ; Store to absolute address
    add d1, [0x80000008]    ; Add from memory
    
    ; ===== Indirect Memory Operands =====
    ld d0, [d1]         ; Load from address in d1
    st [d2], d0         ; Store to address in d2
    add d0, [d1]        ; Add value from address in d1
    
    ; ===== Mixed Operand Types =====
    ; Register + Immediate
    ld d0, 0x2000
    add d0, 0x100
    
    ; Register + Direct Memory
    ld d0, [0x80000000]
    add d0, d1
    
    ; Register + Indirect Memory
    ld d0, [d1]
    add d0, d2
    
    ; ===== Complex Instruction Sequences =====
loop:
    ; Setup: Load base address
    ld d0, 0x80000000
    
    ; Load value from direct address
    ld d1, [0x80000000]
    
    ; Load value from indirect address (address in d0)
    ld d2, [d0]
    
    ; Perform arithmetic
    add d1, 0x1
    add d2, d1
    
    ; Store results
    st [0x80000000], d1     ; Direct store
    st [d0], d2             ; Indirect store
    
    ; Compare and loop
    cmp d1, 0x100
    dec w0

helper:
    ; Helper function demonstrating all operand combinations
    ld l0, 42           ; Immediate to register
    ld l1, [l2]         ; Indirect to register
    add l0, l1          ; Register to register
    st [l2], l0         ; Register to indirect
    
done:
    halt
