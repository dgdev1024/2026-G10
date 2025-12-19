; Test 5: Jump and Call Instructions - 1 and 2 operand forms
; This test verifies that JMP, JP, JPB, JR, and CALL instructions
; work with both 1 operand (implied NC condition) and 2 operands
; (explicit condition code).

.ORG 0x6000

.GLOBAL test_jumps, loop_start

test_jumps:
    ; JMP instruction - 1 operand (implied NC)
    JMP loop_start          ; Jump unconditionally to label
    JMP 0x8000              ; Jump unconditionally to address

    ; JMP instruction - 2 operands (explicit condition)
    JMP NC, loop_start      ; Jump if no carry
    JMP ZS, end_test        ; Jump if zero set
    JMP CS, 0x9000          ; Jump if carry set
    JMP CC, error_handler   ; Jump if carry clear
    JMP VS, overflow_case   ; Jump if overflow set
    JMP VC, normal_case     ; Jump if overflow clear

    ; JP instruction (alias for JMP)
    JP loop_start           ; Jump unconditionally
    JP ZC, done             ; Jump if zero clear

    ; JPB instruction - relative jump with byte offset
    JPB loop_start          ; Relative jump (1 operand)
    JPB ZS, loop_start      ; Conditional relative jump (2 operands)
    JPB NC, back_label      ; Jump to label
    JPB CC, forward_label   ; Jump to label

    ; JR instruction (alias for JPB)
    JR loop_start           ; Relative jump
    JR ZC, loop_start       ; Conditional relative jump

    ; CALL instruction - 1 operand (implied NC)
    CALL subroutine1        ; Call unconditionally
    CALL 0xA000             ; Call to absolute address

    ; CALL instruction - 2 operands (explicit condition)
    CALL NC, subroutine1    ; Call if no carry
    CALL ZS, subroutine2    ; Call if zero set
    CALL CS, error_routine  ; Call if carry set
    CALL VS, overflow_fix   ; Call if overflow set

loop_start:
    INC D0
    CMP D0, 100
    JPB ZC, loop_start      ; Loop while not zero
    RET

back_label:
forward_label:
subroutine1:
    PUSH D0
    ; Do work
    POP D0
    RET

subroutine2:
    RET

error_handler:
error_routine:
overflow_case:
overflow_fix:
normal_case:
done:
end_test:
    HALT
