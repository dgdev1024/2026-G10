; Test 4: Comprehensive Instruction Coverage
; This test covers all instruction types from the G10 CPU ISA to ensure
; the lexer correctly tokenizes every mnemonic.

.ORG 0x5000

test_all_instructions:
    ; CPU Control Instructions
    NOP                 ; No operation
    STOP                ; Enter STOP state (ultra-low power)
    HALT                ; Enter HALT state (low power)
    DI                  ; Disable interrupts
    EI                  ; Enable interrupts (after next instruction)
    EII                 ; Enable interrupts immediately

    ; Flag Manipulation Instructions
    DAA                 ; Decimal adjust accumulator
    scf                 ; Set carry flag
    CCF                 ; Complement carry flag
    CLV                 ; Clear overflow flag
    SEV                 ; Set overflow flag

    ; Load/Store Instructions
    LD D0, D1           ; Load register from register
    LD W2, [$8000]      ; Load word from memory
    LD L3, 42           ; Load immediate value
    LDQ W4, [$FFF0]     ; Load quick (relative to 0xFFFF0000)
    LDP L5, [$80]       ; Load port (relative to 0xFFFFFF00)

    ST [$8100], D6      ; Store register to memory
    ST [$8200], W7      ; Store word to memory
    STQ [$FFE0], L8     ; Store quick
    STP [$90], L9       ; Store port

    ; Register Move Instructions
    MV D10, d11         ; Move register to register
    MWH W12, D13        ; Move word high (Move `W12` into high 16 bits of `D13`)
    MWL D14, W15        ; Move word low (Move high 16 bits of `D14` into `W15`)

    ; Stack Operations
    LSP $8000           ; Load $8000 into stack pointer
    SSP [$8000]         ; Store stack pointer to memory
    PUSH D0             ; Push register to stack
    PUSH W1             ; Push word to stack
    POP D2              ; Pop from stack to register
    POP W3              ; Pop from stack to word register
    SPO D4              ; Stack pointer out (move `SP` into `D4`)
    SPI D5              ; Stack pointer in (move `D5` into `SP`)

    ; Jump and Branch Instructions
    JMP $9000           ; Jump to address (same as `JMP NC, $9000`)
    JMP D6              ; Jump to address in register (same as `JMP NC, D6`)
    JPB NC, loop_start  ; Jump by offset (unconditional)
    JPB ZS, zero_set    ; Jump if zero flag set
    JPB ZC, zero_clear  ; Jump if zero flag clear
    JPB CS, carry_set   ; Jump if carry flag set
    JPB CC, carry_clear ; Jump if carry flag clear
    JPB VS, ovf_set     ; Jump if overflow flag set
    JPB VC, ovf_clear   ; Jump if overflow flag clear

    ; Aliases for jump instructions
    JP 0xA000           ; Alias for `JMP 0xA000` (same as `JMP NC, 0xA000`)
    JR NC, loop_start   ; Alias for `JPB NC, loop_start`

    ; Subroutine Instructions
    CALL 0xB000         ; Call subroutine at address (same as `CALL NC, 0xB000`)
    RET                 ; Return from subroutine (same as `RET NC`, unconditional)
    INT 5               ; Software interrupt
    RETI                ; Return from interrupt

    ; Arithmetic Instructions
    ADD D0, D1          ; Add register `D1` to double word accumulator `D0`
    ADD W0, 100         ; Add immediate to word register `W0`
    ADC D0, D4          ; Add with carry
    SUB D0, D6          ; Subtract
    SBC W0, W8          ; Subtract with carry
    INC D9              ; Increment register
    INC W10             ; Increment word register
    DEC L11             ; Decrement register
    DEC H12             ; Decrement half-register

    ; Logical Instructions
    AND D0, D1          ; Bitwise AND
    AND W0, 0xFF        ; AND with immediate
    OR D0, D4           ; Bitwise OR
    OR L0, 0x0F         ; OR with immediate
    XOR D0, D7          ; Bitwise XOR
    NOT D8              ; Bitwise NOT
    NOT W9              ; NOT word register

    ; Comparison Instructions
    CMP D0, D1          ; Compare registers
    CMP W0, 256         ; Compare with immediate
    CP D0, D4           ; Alias for CMP

    ; Shift Instructions
    SLA D0              ; Shift left arithmetic
    SLA W1              ; Shift left word
    SRA D2              ; Shift right arithmetic
    SRL D3              ; Shift right logical

    ; Swap Instruction
    SWAP D4             ; Swap high and low halves

    ; Rotate Instructions
    RLA                 ; Rotate accumulator left through carry
    RL D5               ; Rotate register left through carry
    RLCA                ; Rotate accumulator left circular
    RLC W6              ; Rotate register left circular
    RRA                 ; Rotate accumulator right through carry
    RR D7               ; Rotate register right through carry
    RRCA                ; Rotate accumulator right circular
    RRC W8              ; Rotate register right circular

    ; Bit Manipulation Instructions
    BIT 0, D9           ; Test bit 0
    BIT 7, L10          ; Test bit 7
    SET 3, W11          ; Set bit 3
    RES 5, D12          ; Reset (clear) bit 5
    TOG 2, L13          ; Toggle bit 2

    ; Special Aliases
    CPL                 ; Complement accumulator (alias for NOT L0)

loop_start:
zero_set:
zero_clear:
carry_set:
carry_clear:
ovf_set:
ovf_clear:
    NOP
    RET
