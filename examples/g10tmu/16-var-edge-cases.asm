; Test: Edge Cases - Negative Numbers and Boundary Values
; This example tests edge cases including negative values,
; zero operations, and boundary conditions.

; Negative number handling (via unary minus)
.let $neg_one = -1              ; -1 = 0xFFFFFFFFFFFFFFFF (in 64-bit)
.let $neg_ten = -10
.let $positive = 50
.let $subtracted = $positive + $neg_ten     ; 50 + (-10) = 40

; Zero operations
.let $zero = 0
.let $zero_mul = 100 * $zero    ; 0
.let $zero_add = $zero + 0      ; 0
.let $zero_or = $zero | 0xFF    ; 0xFF = 255
.let $zero_and = 0xFF & $zero   ; 0

; Division edge cases (non-zero divisor)
.let $div_exact = 100 / 10      ; 10
.let $div_truncate = 17 / 5     ; 3 (integer division truncates)
.let $mod_result = 17 % 5       ; 2

; Shift edge cases
.let $shift_zero = 0xFF << 0    ; No shift = 255
.let $shift_full = 1 << 8       ; 256

; Large values fitting in different sizes
.let $byte_val = 0x7F           ; 127 (max signed byte)
.let $word_val = 0x7FFF         ; 32767 (max signed word)

; Reassignment chain
.let $x = 1
$x = $x + $x        ; 2
$x = $x * $x        ; 4
$x = $x * $x        ; 16
$x = $x * $x        ; 256

; Expected values:
;   subtracted = 40
;   zero_mul = 0
;   zero_or = 255
;   zero_and = 0
;   div_exact = 10
;   div_truncate = 3
;   mod_result = 2
;   shift_zero = 255
;   shift_full = 256
;   x = 256

.global main
.ram
    r_subtracted:   .byte 1
    r_zero_mul:     .byte 1
    r_zero_or:      .byte 1
    r_zero_and:     .byte 1
    r_div_exact:    .byte 1
    r_div_trunc:    .byte 1
    r_mod:          .byte 1
    r_shift_zero:   .byte 1
    r_shift_full:   .word 1
    r_x:            .word 1
.rom
    main:
        ld l0, $subtracted
        st [r_subtracted], l0
        
        ld l0, $zero_mul
        st [r_zero_mul], l0
        
        ld l0, $zero_or
        st [r_zero_or], l0
        
        ld l0, $zero_and
        st [r_zero_and], l0
        
        ld l0, $div_exact
        st [r_div_exact], l0
        
        ld l0, $div_truncate
        st [r_div_trunc], l0
        
        ld l0, $mod_result
        st [r_mod], l0
        
        ld l0, $shift_zero
        st [r_shift_zero], l0
        
        ld w0, $shift_full
        st [r_shift_full], w0
        
        ld w0, $x
        st [r_x], w0
        
        stop
