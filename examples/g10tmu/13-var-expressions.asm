; Test: Complex Expressions with Variables
; This example tests arithmetic expressions involving variables,
; nested parentheses, and variable chaining.

; Basic arithmetic expressions
.let $a = 10
.let $b = 20
.let $c = $a + $b                   ; 30
.let $d = $b - $a                   ; 10
.let $e = $a * $b                   ; 200
.let $f = $b / $a                   ; 2
.let $g = $b % 7                    ; 6 (20 mod 7)

; Nested parentheses
.let $nested = (($a + $b) * 2) - 10 ; ((10 + 20) * 2) - 10 = 50
.let $complex = ($a * ($b + 5)) / 5 ; (10 * (20 + 5)) / 5 = 50

; Chained expressions using previous results
.let $chain1 = $c + $d              ; 30 + 10 = 40
.let $chain2 = $chain1 * 2          ; 40 * 2 = 80

; Bitwise operations
.let $bits_and = 0xFF & 0x0F        ; 0x0F = 15
.let $bits_or  = 0xF0 | 0x0F        ; 0xFF = 255
.let $bits_xor = 0xFF ^ 0xAA        ; 0x55 = 85
.let $bits_shl = 1 << 4             ; 16
.let $bits_shr = 256 >> 4           ; 16

; Mixed arithmetic and bitwise
.let $mixed = (($a + $b) << 2) | 3  ; ((30) << 2) | 3 = 120 | 3 = 123

; Expected values to store in RAM:
;   c = 30, d = 10, e = 200, f = 2, g = 6
;   nested = 50, complex = 50
;   chain1 = 40, chain2 = 80
;   bits_and = 15, bits_or = 255, bits_xor = 85
;   bits_shl = 16, bits_shr = 16
;   mixed = 123

.global main
.ram
    result_c:       .byte 1
    result_d:       .byte 1
    result_e:       .byte 1
    result_f:       .byte 1
    result_g:       .byte 1
    result_nested:  .byte 1
    result_complex: .byte 1
    result_chain1:  .byte 1
    result_chain2:  .byte 1
    result_and:     .byte 1
    result_or:      .byte 1
    result_xor:     .byte 1
    result_shl:     .byte 1
    result_shr:     .byte 1
    result_mixed:   .byte 1
.rom
    main:
        ; Store arithmetic results
        ld l0, $c
        st [result_c], l0
        ld l0, $d
        st [result_d], l0
        ld l0, $e
        st [result_e], l0
        ld l0, $f
        st [result_f], l0
        ld l0, $g
        st [result_g], l0
        
        ; Store nested/complex results
        ld l0, $nested
        st [result_nested], l0
        ld l0, $complex
        st [result_complex], l0
        
        ; Store chained results
        ld l0, $chain1
        st [result_chain1], l0
        ld l0, $chain2
        st [result_chain2], l0
        
        ; Store bitwise results
        ld l0, $bits_and
        st [result_and], l0
        ld l0, $bits_or
        st [result_or], l0
        ld l0, $bits_xor
        st [result_xor], l0
        ld l0, $bits_shl
        st [result_shl], l0
        ld l0, $bits_shr
        st [result_shr], l0
        ld l0, $mixed
        st [result_mixed], l0
        
        stop
