; Test 18: Complex Expressions with Full Precedence
; Tests the parser's ability to correctly parse complex expressions
; involving multiple operators at different precedence levels.

.org 0x2000

; Full precedence chain (lowest to highest):
;   | (bitwise OR)
;   ^ (bitwise XOR)
;   & (bitwise AND)
;   <<, >> (shift)
;   +, - (additive)
;   *, /, % (multiplicative)
;   ** (exponentiation)
;   -, ~, ! (unary)
;   () (grouping), literals, identifiers

; Complex expression combining all binary operators
test_full_precedence:
    ; This parses as: 1 | (2 ^ (3 & (4 << (5 + (6 * 7)))))
    ld d0, 1 | 2 ^ 3 & 4 << 5 + 6 * 7

; Verify precedence with equivalent grouped expressions
test_equivalent_grouped:
    ; These should be equivalent if precedence is correct:
    ld d0, 2 + 3 * 4                ; = 2 + (3 * 4) = 2 + 12 = 14
    ld d0, (2 + 3) * 4              ; = 5 * 4 = 20 (different!)
    
    ld d0, 1 << 2 + 3               ; = 1 << (2 + 3) = 1 << 5 = 32
    ld d0, (1 << 2) + 3             ; = 4 + 3 = 7 (different!)
    
    ld d0, 0xFF & 0x0F | 0xF0       ; = (0xFF & 0x0F) | 0xF0 = 0x0F | 0xF0 = 0xFF
    ld d0, 0xFF & (0x0F | 0xF0)     ; = 0xFF & 0xFF = 0xFF (same result by chance)

; Mixed unary and binary
test_unary_binary_mix:
    ld d0, -1 + 2                   ; (-1) + 2 = 1
    ld d0, 1 + -2                   ; 1 + (-2) = -1
    ld d0, ~0 + 1                   ; (~0) + 1 = -1 + 1 = 0 (two's complement)
    ld d0, ~(0 + 1)                 ; ~1 = -2
    
    ld d0, -2 * 3                   ; (-2) * 3 = -6
    ld d0, 2 * -3                   ; 2 * (-3) = -6
    ld d0, -2 * -3                  ; (-2) * (-3) = 6
    
    ld d0, ~0xFF & 0xFF             ; (~0xFF) & 0xFF
    ld d0, ~(0xFF & 0xFF)           ; ~0xFF

; Deeply nested groupings
test_deep_nesting:
    ld d0, ((((1))))
    ld d0, ((1 + 2))
    ld d0, (((1 + 2) * 3))
    ld d0, ((((1 + 2) * 3) - 4))
    ld d0, (((((1 + 2) * 3) - 4) / 5))

; Right-associativity of exponentiation in complex expressions
test_exponent_complex:
    ld d0, 2 ** 3 ** 2              ; 2 ** (3 ** 2) = 2 ** 9 = 512
    ld d0, 1 + 2 ** 3               ; 1 + (2 ** 3) = 1 + 8 = 9
    ld d0, 2 * 3 ** 2               ; 2 * (3 ** 2) = 2 * 9 = 18
    ld d0, 2 ** 3 + 1               ; (2 ** 3) + 1 = 8 + 1 = 9

; Bitwise operations building flag values
test_flag_building:
    ld d0, 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3  ; = 0x0F
    ld d0, 0xFF & ~(1 << 3)         ; Clear bit 3: 0xFF & ~0x08 = 0xF7
    ld d0, 0x00 | 1 << 7            ; Set bit 7: 0x80

; Data with complex expressions
.org 0x80000000
complex_data:
.byte 1 + 2 * 3, (1 + 2) * 3
.word 0xFF & 0x0F | 0xF0, 1 << 8 | 0xFF
.dword 0xDEAD << 16 | 0xBEEF, ~0 + 1
