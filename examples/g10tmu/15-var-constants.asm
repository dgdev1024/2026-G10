; Test: Constants and Their Usage
; This example tests constants defined from literals, expressions,
; and other constants/variables.

; Constants from literals
.const $BYTE_MAX = 0xFF
.const $WORD_MAX = 0xFFFF
.const $MAGIC = 0xDEAD

; Constants from expressions
.const $KILO = 1024
.const $MEGA = $KILO * $KILO        ; 1,048,576
.const $HALF_KILO = $KILO / 2       ; 512
.const $KILO_PLUS_ONE = $KILO + 1   ; 1025

; Constants derived from other constants
.const $DOUBLE_MAGIC = $MAGIC * 2   ; 0x1BD5A
.const $MAGIC_LOW = $MAGIC & 0xFF   ; 0xAD = 173
.const $MAGIC_HIGH = ($MAGIC >> 8) & 0xFF   ; 0xDE = 222

; Variable that captures constant at a point in time
.let $counter = 0
.const $COUNTER_AT_ZERO = $counter  ; 0
$counter = 100
.const $COUNTER_AT_100 = $counter   ; 100
$counter = 200
; Note: $COUNTER_AT_ZERO and $COUNTER_AT_100 are frozen at their definition time

; Using constants in expressions with variables
.let $base = $BYTE_MAX              ; 255
$base -= $MAGIC_LOW                 ; 255 - 173 = 82

; Expected RAM values:
;   byte_max = 0xFF = 255
;   half_kilo low byte = 0x00, high byte = 0x02 (512 = 0x0200)
;   magic_low = 173
;   magic_high = 222
;   counter_at_zero = 0
;   counter_at_100 = 100
;   base = 82

.global main
.ram
    r_byte_max:         .byte 1
    r_half_kilo:        .word 1
    r_magic_low:        .byte 1
    r_magic_high:       .byte 1
    r_counter_zero:     .byte 1
    r_counter_100:      .byte 1
    r_base:             .byte 1
.rom
    main:
        ld l0, $BYTE_MAX
        st [r_byte_max], l0
        
        ld w0, $HALF_KILO
        st [r_half_kilo], w0
        
        ld l0, $MAGIC_LOW
        st [r_magic_low], l0
        
        ld l0, $MAGIC_HIGH
        st [r_magic_high], l0
        
        ld l0, $COUNTER_AT_ZERO
        st [r_counter_zero], l0
        
        ld l0, $COUNTER_AT_100
        st [r_counter_100], l0
        
        ld l0, $base
        st [r_base], l0
        
        stop
