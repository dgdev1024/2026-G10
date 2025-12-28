; Test: Practical Use Case - Configuration Constants and Computed Values
; This example demonstrates a practical use of variables and constants
; for configuration and computed values in a real program.

; === Configuration Constants ===
; These would typically be changed to configure the program
.const $BUFFER_SIZE = 8
.const $START_VALUE = 0x10
.const $INCREMENT = 5

; === Computed Constants ===
; These are derived from configuration
.const $BUFFER_END = $BUFFER_SIZE - 1
.const $LAST_VALUE = $START_VALUE + ($INCREMENT * $BUFFER_END)
.const $BUFFER_MASK = $BUFFER_SIZE - 1      ; For power-of-2 wrapping

; === Build-time Loop Unrolling ===
; Use variables to compute values for each iteration
.let $i = 0
.let $val = $START_VALUE

; Iteration 0
.const $VAL_0 = $val
$val += $INCREMENT
$i += 1

; Iteration 1
.const $VAL_1 = $val
$val += $INCREMENT
$i += 1

; Iteration 2
.const $VAL_2 = $val
$val += $INCREMENT
$i += 1

; Iteration 3
.const $VAL_3 = $val
$val += $INCREMENT
$i += 1

; Iteration 4
.const $VAL_4 = $val
$val += $INCREMENT
$i += 1

; Iteration 5
.const $VAL_5 = $val
$val += $INCREMENT
$i += 1

; Iteration 6
.const $VAL_6 = $val
$val += $INCREMENT
$i += 1

; Iteration 7
.const $VAL_7 = $val

; Verify computed values:
;   VAL_0 = 0x10 = 16
;   VAL_1 = 0x15 = 21
;   VAL_2 = 0x1A = 26
;   VAL_3 = 0x1F = 31
;   VAL_4 = 0x24 = 36
;   VAL_5 = 0x29 = 41
;   VAL_6 = 0x2E = 46
;   VAL_7 = 0x33 = 51

; === Program ===
.global main
.ram
    buffer:     .byte $BUFFER_SIZE      ; Allocate buffer using constant
.rom
    main:
        ; Fill buffer with pre-computed values (loop unrolled)
        ld l0, $VAL_0
        st [buffer + 0], l0
        
        ld l0, $VAL_1
        st [buffer + 1], l0
        
        ld l0, $VAL_2
        st [buffer + 2], l0
        
        ld l0, $VAL_3
        st [buffer + 3], l0
        
        ld l0, $VAL_4
        st [buffer + 4], l0
        
        ld l0, $VAL_5
        st [buffer + 5], l0
        
        ld l0, $VAL_6
        st [buffer + 6], l0
        
        ld l0, $VAL_7
        st [buffer + 7], l0
        
        stop
