; Test 09: Stack Operations
; Tests PUSH, POP (with D registers only), CALL, RET.
; Note: PUSH/POP only work with 32-bit D registers.
;
; Expected RAM layout at $80000000:
;   $00-$03: 0xDEADBEEF - PUSH/POP dword worked
;   $04: 0xAA          - CALL/RET worked (value from subroutine)
;   $05: 0xBB          - Nested CALL worked (value from nested sub)

.global main

; RAM section for test results
.org 0x80000000
    result_push_dword:  .dword 1
    result_call:        .byte 1
    result_nested:      .byte 1

; Code section
.org 0x2000
main:
    ; Test PUSH/POP with dword (D register - the only size supported)
    ld d0, 0xDEADBEEF
    push d0             ; Push 0xDEADBEEF onto stack
    ld d0, 0x00000000   ; Clear D0
    pop d0              ; Pop back into D0
    st [result_push_dword], d0
    
    ; Test CALL/RET
    call test_sub1
    st [result_call], l0  ; L0 should contain 0xAA from subroutine
    
    ; Test nested CALL
    call test_sub2
    st [result_nested], l0  ; L0 should contain 0xBB from nested sub
    
    ; End program
    stop

; Subroutine 1: Simple return with value
test_sub1:
    ld l0, 0xAA
    ret

; Subroutine 2: Calls another subroutine
test_sub2:
    call test_sub3
    ret

; Subroutine 3: Returns a value
test_sub3:
    ld l0, 0xBB
    ret
