; =============================================================================
; Test 6: Bitwise Operations (Single File)
; =============================================================================
; Tests bitwise AND, OR, XOR, NOT, and bit manipulation instructions.
; =============================================================================

.org $80000000
and_result:
    .byte 1
or_result:
    .byte 1
xor_result:
    .byte 1
not_result:
    .byte 1
shift_result:
    .byte 1

.org $2000

main:
    ; Test AND operation: $F0 AND $0F = $00
    ld l0, $F0
    and l0, $0F
    st [and_result], l0         ; Should be $00
    
    ; Test OR operation: $F0 OR $0F = $FF
    ld l0, $F0
    or l0, $0F
    st [or_result], l0          ; Should be $FF
    
    ; Test XOR operation: $AA XOR $55 = $FF
    ld l0, $AA
    xor l0, $55
    st [xor_result], l0         ; Should be $FF
    
    ; Test NOT operation
    ld l0, $AA
    not l0                      ; NOT $AA = $55
    st [not_result], l0         ; Should be $55
    
    ; Test bit shifts
    ld l0, $01                  ; 00000001
    sla l0                      ; Shift left: 00000010
    sla l0                      ; Shift left: 00000100
    sla l0                      ; Shift left: 00001000 ($08)
    st [shift_result], l0       ; Should be $08
    
    ; Test bit set/reset/toggle
    ld l1, $00
    set 3, l1                   ; Set bit 3: 00001000
    set 7, l1                   ; Set bit 7: 10001000
    res 3, l1                   ; Reset bit 3: 10000000
    tog 0, l1                   ; Toggle bit 0: 10000001
    
    halt
