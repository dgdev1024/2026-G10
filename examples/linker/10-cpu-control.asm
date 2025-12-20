; =============================================================================
; Test 10: CPU Control Instructions (Single File)
; =============================================================================
; Tests NOP, HALT, and flag manipulation instructions.
; Note: STOP, DI, EI, EII, RETI are not easily testable without interrupt setup.
; =============================================================================

.org $80000000
flag_test:
    .byte 1

.org $2000

main:
    ; Test NOP (no operation)
    nop
    nop
    nop
    
    ; Test flag manipulation
    ; Set carry flag
    scf                         ; C = 1
    
    ; Complement (toggle) carry flag
    ccf                         ; C = 0 (was 1)
    ccf                         ; C = 1 (was 0)
    
    ; Test overflow flag
    clv                         ; V = 0 (clear overflow)
    sev                         ; V = 1 (set overflow)
    clv                         ; V = 0 (clear again)
    
    ; Test DAA (Decimal Adjust Accumulator)
    ; Add two BCD numbers: 29 + 47 = 76
    ld l0, $29                  ; L0 = $29 (BCD 29)
    ld l1, $47                  ; L1 = $47 (BCD 47)
    add l0, l1                  ; L0 = $70 (binary result)
    daa                         ; L0 = $76 (BCD adjusted)
    
    st [flag_test], l0
    
    ; End with HALT
    halt
