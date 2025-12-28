; Test: All Compound Assignment Operators
; This example tests every compound assignment operator.

; Start with a base value
.let $val = 100

; Simple assignment (reset)
$val = 50

; Addition assignment
$val += 10          ; 50 + 10 = 60

; Subtraction assignment
$val -= 5           ; 60 - 5 = 55

; Store intermediate result
.const $AFTER_ADD_SUB = $val    ; 55

; Multiplication assignment
$val *= 2           ; 55 * 2 = 110

; Division assignment
$val /= 11          ; 110 / 11 = 10

; Modulo assignment
$val = 47
$val %= 10          ; 47 % 10 = 7

; Store intermediate result
.const $AFTER_MUL_DIV_MOD = $val    ; 7

; Bitwise AND assignment
$val = 0xFF
$val &= 0x0F        ; 0xFF & 0x0F = 0x0F = 15

; Bitwise OR assignment
$val |= 0xF0        ; 0x0F | 0xF0 = 0xFF = 255

; Bitwise XOR assignment
$val ^= 0xAA        ; 0xFF ^ 0xAA = 0x55 = 85

; Store intermediate result
.const $AFTER_BITWISE = $val    ; 85

; Shift left assignment
$val = 1
$val <<= 7          ; 1 << 7 = 128

; Shift right assignment
$val >>= 3          ; 128 >> 3 = 16

; Store intermediate result
.const $AFTER_SHIFT = $val      ; 16

; Exponentiation assignment
$val = 2
$val **= 8          ; 2^8 = 256

.const $AFTER_EXPONENT = $val   ; 256 (stored as word)

; Expected values:
;   AFTER_ADD_SUB = 55
;   AFTER_MUL_DIV_MOD = 7
;   AFTER_BITWISE = 85 (0x55)
;   AFTER_SHIFT = 16
;   AFTER_EXPONENT = 256 (0x100)

.global main
.ram
    r_add_sub:  .byte 1
    r_mdm:      .byte 1
    r_bitwise:  .byte 1
    r_shift:    .byte 1
    r_exp:      .word 1     ; 16-bit for 256
.rom
    main:
        ld l0, $AFTER_ADD_SUB
        st [r_add_sub], l0
        
        ld l0, $AFTER_MUL_DIV_MOD
        st [r_mdm], l0
        
        ld l0, $AFTER_BITWISE
        st [r_bitwise], l0
        
        ld l0, $AFTER_SHIFT
        st [r_shift], l0
        
        ld w0, $AFTER_EXPONENT
        st [r_exp], w0
        
        stop
