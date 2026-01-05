; Test 8: Trigonometric Functions - sin(), cos()
; This test verifies sin() and cos() functions using turns.
; 1.0 turn = 360 degrees = 2*PI radians

.org 0x0000

; sin(0.0 turns) = 0
; sin(0.25 turns) = sin(90°) = 1
; sin(0.5 turns) = sin(180°) = 0
; cos(0.0 turns) = 1
; cos(0.25 turns) = cos(90°) = 0
; cos(0.5 turns) = cos(180°) = -1

.byte {round(fmul(sin(0.0), 100.0))}      ; sin(0) = 0, *100 = 0
.byte {round(fmul(sin(0.25), 100.0))}     ; sin(90°) = 1, *100 = 100
.byte {round(fmul(sin(0.5), 100.0))}      ; sin(180°) = 0, *100 = 0
.byte {round(fmul(cos(0.0), 100.0))}      ; cos(0) = 1, *100 = 100
.byte {round(fmul(cos(0.25), 100.0))}     ; cos(90°) = 0, *100 = 0
.byte {round(fmul(cos(0.5), 100.0))}      ; cos(180°) = -1, *100 = -100

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Trigonometric functions sin() and cos()
;
; This test verifies:
; - sin() returns sine of angle in turns
; - cos() returns cosine of angle in turns
; Values are multiplied by 100 and rounded for integer comparison.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 100
; .byte 0
; .byte 100
; .byte 0
; .byte -100
; halt
;
;----------------------------------------------------------------------------
