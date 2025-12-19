; Test 6: Edge Cases - Stress testing the parser implementation
; This test verifies the parser handles various edge cases correctly.

.ORG 0x0000

; Edge Case 1: Multiple labels on consecutive lines
label1:
label2:
label3:
    NOP

; Edge Case 2: Label immediately followed by directive
data_start:
.BYTE 0x00

; Edge Case 3: Label immediately followed by another label and directive
section1:
section2:
.ORG 0x1000

; Edge Case 4: Instructions with all register size combinations
test_registers:
    ; 32-bit registers
    LD D0, D1
    MV D15, D14
    ADD D0, D2
    
    ; 16-bit registers
    LD W0, W1
    MV W15, W14
    
    ; 8-bit high byte registers
    LD H0, H1
    MV H15, H14
    
    ; 8-bit low byte registers
    LD L0, L1
    MV L15, L14
    
    ; Mixed size operations
    MV H0, L1
    MV L2, H3

; Edge Case 5: All condition codes in various instructions
test_conditions:
    JMP NC, target
    JMP ZS, target
    JMP ZC, target
    JMP CS, target
    JMP CC, target
    JMP VS, target
    JMP VC, target
    
    RET NC
    RET ZS
    RET ZC
    RET CS
    RET CC
    RET VS
    RET VC

; Edge Case 6: Maximum values for different data sizes
test_max_values:
    .BYTE 0xFF                          ; Max 8-bit
    .WORD 0xFFFF                        ; Max 16-bit
    .DWORD 0xFFFFFFFF                   ; Max 32-bit
    .BYTE 255                           ; Max 8-bit decimal
    .WORD 65535                         ; Max 16-bit decimal
    .DWORD 4294967295                   ; Max 32-bit decimal

; Edge Case 7: Minimum and zero values
test_min_values:
    .BYTE 0, 0x00
    .WORD 0, 0x0000
    .DWORD 0, 0x00000000

; Edge Case 8: Multiple operands in data directives
test_multiple_data:
    .BYTE 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    .WORD 0x1111, 0x2222, 0x3333, 0x4444
    .DWORD 0x11111111, 0x22222222, 0x33333333

; Edge Case 9: String literals with special characters
test_strings:
    .BYTE "Hello, World!", 0
    .BYTE "TestNewline", 0
    .BYTE "PathFile", 0
    .BYTE "QuoteTest", 0
    .BYTE "", 0                         ; Empty string

; Edge Case 10: Character literals
test_characters:
    .BYTE 'A', 'Z', '0', '9'
    .BYTE ' '

; Edge Case 11: All addressing modes
test_addressing:
    ; Direct register
    LD D0, D1
    
    ; Immediate values
    LD D0, 42
    LD W0, 0x1234
    LD L0, 0b11110000
    
    ; Direct memory addressing
    LD D0, [0x8000]
    ST [0x9000], D1
    
    ; Indirect register addressing
    LD D0, [D1]
    ST [D2], D3
    LD W0, [W1]
    
    ; Label references
    JMP target
    CALL subroutine
    
; Edge Case 12: Zero-operand instructions
test_zero_operands:
    NOP
    HALT
    STOP
    DI
    EI
    EII
    DAA
    SCF
    CCF
    CLV
    SEV
    RETI
    RLA
    RLCA
    RRA
    RRCA
    CPL

; Edge Case 13: One-operand instructions
test_one_operand:
    PUSH D0
    POP D1
    INC D2
    DEC D3
    NOT D4
    SLA D5
    SRA D6
    SRL D7
    SWAP D8
    RL D9
    RLC D10
    RR D11
    RRC D12
    INT 0xFF
    LSP D13
    SSP D14
    SPO D15
    SPI W0

; Edge Case 14: Two-operand instructions (all ALU operations)
test_two_operands:
    ; Arithmetic (must use accumulator D0/W0/L0)
    ADD D0, D1
    ADC D0, D2
    SUB D0, D3
    SBC D0, D4
    
    ; Logical (must use accumulator)
    AND D0, D5
    OR D0, D6
    XOR D0, D7
    
    ; Comparison (must use accumulator)
    CMP D0, D8
    CP D0, D9
    
    ; Load/Store/Move (any registers)
    LD D10, D11
    ST [D12], D13
    MV D14, D15

; Edge Case 15: Bit operations
test_bit_ops:
    BIT 0, D0
    BIT 7, L1
    SET 3, W2
    RES 5, H3
    TOG 1, D4

; Edge Case 16: Jump/Call variations
test_jumps_calls:
    ; Unconditional (1 operand)
    JMP target
    JP target
    JPB target
    JR target
    CALL subroutine
    
    ; Conditional (2 operands)
    JMP NC, target
    CALL ZS, subroutine
    JPB CC, target
    JR VS, target

; Edge Case 17: Return variations
test_returns:
    RET                 ; Unconditional (0 operands)
    RET NC              ; Conditional (1 operand)
    RET ZS
    RETI                ; Interrupt return (0 operands only)

; Edge Case 18: Global and extern symbols
.GLOBAL label1, label2, test_registers
.EXTERN external_func1, external_func2, external_data

; Edge Case 19: Long identifier names
very_long_label_name_to_test_identifier_length_limits:
    LD D0, 1
    JMP very_long_label_name_to_test_identifier_length_limits

; Edge Case 20: Adjacent labels and statements
compact:
    LD D0, 0
    ADD D0, 1
    RET

; Edge Case 21: Instruction aliases
test_aliases:
    JP target           ; Alias for JMP
    JR target           ; Alias for JPB
    CP D0, D1           ; Alias for CMP

; Edge Case 22: Stack operations
test_stack:
    PUSH D0
    PUSH W1
    PUSH L2
    POP L3
    POP W4
    POP D5

; Edge Case 23: Accumulator validation
test_accumulator_requirement:
    ; These must use D0, W0, or L0 as first operand
    ADD D0, 5
    ADD W0, 10
    ADD L0, 3
    SUB D0, D1
    AND W0, W1
    XOR L0, L1
    CMP D0, 100

; Edge Case 24: Maximum register indices (0-15)
test_max_register_indices:
    LD D15, 0
    LD W15, 0
    LD H15, 0
    LD L15, 0
    MV D0, D15
    MV W0, W15
    MV H0, H15
    MV L0, L15

; Edge Case 25: Binary, octal, and hex literals
test_number_bases:
    LD D0, 0b11111111           ; Binary
    LD D1, 0o377                ; Octal
    LD D2, 0xFF                 ; Hexadecimal
    LD D3, 255                  ; Decimal
    LD D4, 0b10101010
    LD D5, 0o252
    LD D6, 0xAA

; Edge Case 26: Mixed case sensitivity (if applicable)
MixedCase:
    nop                         ; Lowercase instruction
    NOP                         ; Uppercase instruction
    Nop                         ; Mixed case instruction

target:
subroutine:
    RET

; Edge Case 27: Comments in various positions
    NOP                         ; Comment after instruction
    ; Comment before instruction
    NOP
NOP                             ; No indentation
    NOP                         ; Standard indentation
        NOP                     ; Extra indentation

; Edge Case 28: Multiple directives in sequence
.ORG 0x2000
.GLOBAL final_label
.EXTERN final_extern

final_label:
    HALT

; Edge Case 29: Boundary address values
boundary_test:
    JMP 0x00000000              ; Min address
    JMP 0xFFFFFFFF              ; Max address
    LD D0, [0x00000000]
    ST [0xFFFFFFFF], D0

; End of edge case tests
