; Test 3: Operand Types - All operand variations
; This test covers all types of operands the parser should handle.

.ORG 0x5000

test_operands:
    ; Register operands
    LD D0, D1           ; Full register to full register
    LD W2, W3           ; Word register to word register
    LD L4, L5           ; Low byte register to low byte register
    MV H6, L7           ; High byte to low byte

    ; Immediate operands
    LD D0, 42           ; Decimal immediate
    LD W1, 0xFF         ; Hex immediate
    LD L2, 0b11110000   ; Binary immediate
    LD L3, 'X'          ; Character immediate

    ; Memory address operands (direct)
    LD D4, [0x8000]     ; Load from absolute address
    ST [0x8100], D5     ; Store to absolute address
    LD W6, [0xFFFF0000] ; Load from high memory

    ; Indirect register operands
    LD L7, [D8]         ; Load from address in D8
    ST [D9], L10        ; Store to address in D9
    LDQ W11, [W12]      ; Load using word register as pointer

    ; Label reference operands
    JMP loop_start
    CALL subroutine
    JPB NC, end_func

loop_start:
    INC D0
    CMP D0, 100
    JPB ZC, loop_start

subroutine:
    RET

end_func:
    HALT
