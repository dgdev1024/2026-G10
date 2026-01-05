; Test 3: Integer Functions - min(), max(), clamp()
; This test verifies the min, max, and clamp functions.

.org 0x0000

.byte {min(10, 20)}         ; Should be 10
.byte {min(20, 10)}         ; Should be 10
.byte {max(10, 20)}         ; Should be 20
.byte {max(20, 10)}         ; Should be 20
.byte {clamp(5, 0, 10)}     ; Should be 5 (in range)
.byte {clamp(-5, 0, 10)}    ; Should be 0 (clamped to lo)
.byte {clamp(15, 0, 10)}    ; Should be 10 (clamped to hi)

halt

;----------------------------------------------------------------------------
;
; TEST CASE: min(), max(), clamp() functions
;
; This test verifies:
; - min() returns the smaller of two values
; - max() returns the larger of two values
; - clamp() constrains a value to a range [lo, hi]
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 10
; .byte 10
; .byte 20
; .byte 20
; .byte 5
; .byte 0
; .byte 10
; halt
;
;----------------------------------------------------------------------------
