; Test 20: Comprehensive Instruction Set
; Tests the parser's ability to handle all major instruction categories.

.org 0x2000

; CPU Control Instructions
test_control:
    nop                     ; No operation
    stop                    ; Stop execution
    halt                    ; Halt CPU
    di                      ; Disable interrupts
    ei                      ; Enable interrupts

; Arithmetic on accumulator
test_accumulator:
    daa                     ; Decimal adjust accumulator
    scf                     ; Set carry flag
    ccf                     ; Complement carry flag
    clv                     ; Clear overflow flag
    sev                     ; Set overflow flag

; Load/Store Instructions
test_load_store:
    ld d0, 0x1234           ; Load immediate
    ld d0, [0x80000000]     ; Load direct
    ld d0, [d1]             ; Load indirect
    st [0x80000000], d0     ; Store direct
    st [d1], d0             ; Store indirect

; Move Instructions
test_move:
    mv d0, d1               ; Move register to register
    mwh d0, w1              ; Move word to high word of dword
    ; mwl does not exist - use mv with word registers instead

; Stack Instructions  
test_stack:
    push d0                 ; Push to stack
    pop d0                  ; Pop from stack
    lsp 0x80000000          ; Load stack pointer with immediate
    ssp [0x80000000]        ; Store stack pointer to address
    spo d0                  ; Stack pointer out to register
    spi d0                  ; Stack pointer in from register

; Jump/Call Instructions
target_label:
test_jumps:
    jmp nc, target_label    ; Jump unconditional
    jpb nc, target_label    ; Jump relative backward
    call nc, target_label   ; Call subroutine
    ret nc                  ; Return from subroutine
    reti nc                 ; Return from interrupt

; Arithmetic Instructions
test_arithmetic:
    inc d0                  ; Increment
    dec d0                  ; Decrement
    add d0, d1              ; Add (32-bit, D0 destination)
    sub d0, d1              ; Subtract (32-bit, D0 destination)
    cmp l0, l1              ; Compare (8-bit only, L0 destination)

; Logical Instructions (8-bit ALU uses L0 as accumulator)
test_logical:
    and l0, l1              ; Bitwise AND
    or l0, l1               ; Bitwise OR
    xor l0, l1              ; Bitwise XOR
    not l0                  ; Bitwise NOT (complement)
    cpl                     ; Complement accumulator

; Shift/Rotate Instructions
test_shift_rotate:
    sla l0                  ; Shift left arithmetic (8-bit)
    sra l0                  ; Shift right arithmetic (8-bit)
    srl l0                  ; Shift right logical (8-bit)
    rla                     ; Rotate left accumulator
    rl l0                   ; Rotate left through carry (8-bit)
    rlca                    ; Rotate left accumulator circular
    rlc l0                  ; Rotate left circular (8-bit)
    rra                     ; Rotate right accumulator
    rr l0                   ; Rotate right through carry (8-bit)
    rrca                    ; Rotate right accumulator circular
    rrc l0                  ; Rotate right circular (8-bit)
    swap d0                 ; Swap words (32-bit)

; Bit Operations (only L registers or [DX] indirect are supported)
test_bit_ops:
    bit 3, l0               ; Test bit 3 in l0
    set 3, l0               ; Set bit 3 in l0
    res 3, l0               ; Reset bit 3 in l0
    tog 3, l0               ; Toggle bit 3 in l0
    bit 7, l1               ; Test bit 7 in l1
    bit 0, [d0]             ; Test bit 0 at address in d0
