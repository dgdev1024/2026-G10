; Test 5: Operators and Punctuation
; This test verifies proper spacing around operators and punctuation.

.org 0x0000

; Arithmetic expressions
ld r0, (1 + 2)
ld r1, (3 * 4)
ld r2, [r0 + r1]

; Comparison and logical operators
ld r3, (r0 == r1)
ld r4, (r0 && r1)
ld r5, (r0 || r1)

; Bitwise operators
ld r6, (r0 & 0xFF)
ld r7, (r1 | 0x0F)
ld r8, (r2 ^ r3)
ld r9, (~r0)
ld r10, (r0 << 4)
ld r11, (r1 >> 2)

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Operators and Punctuation
;
; This test verifies that the preprocessor correctly handles spacing around
; various operators and punctuation marks. Parentheses and brackets should
; not have spaces immediately after opening or before closing.
;
; EXPECTED OUTPUT:
;
; .org 0x0000
; ld r0, (1 + 2)
; ld r1, (3 * 4)
; ld r2, [r0 + r1]
; ld r3, (r0 == r1)
; ld r4, (r0 && r1)
; ld r5, (r0 || r1)
; ld r6, (r0 & 0xFF)
; ld r7, (r1 | 0x0F)
; ld r8, (r2 ^ r3)
; ld r9, (~r0)
; ld r10, (r0 << 4)
; ld r11, (r1 >> 2)
; halt
;
;----------------------------------------------------------------------------
