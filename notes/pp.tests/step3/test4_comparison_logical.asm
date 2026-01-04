; Test 4: Comparison and Logical Operations
; This test verifies comparison and logical operator evaluation.

.org 0x0000

    ld r0, {5 < 10}             ; Less than: Should be 1 (true)
    ld r1, {10 <= 10}           ; Less or equal: Should be 1 (true)
    ld r2, {15 > 10}            ; Greater than: Should be 1 (true)
    ld r3, {5 >= 10}            ; Greater or equal: Should be 0 (false)
    ld r4, {5 == 5}             ; Equal: Should be 1 (true)
    ld r5, {5 != 10}            ; Not equal: Should be 1 (true)
    ld r6, {1 && 1}             ; Logical AND: Should be 1
    ld r7, {0 || 1}             ; Logical OR: Should be 1
    ld r8, {!0}                 ; Logical NOT: Should be 1
    ld r9, {!1}                 ; Logical NOT: Should be 0
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Comparison and Logical Operations
;
; This test verifies that comparison (<, <=, >, >=, ==, !=) and
; logical (&&, ||, !) operations are correctly evaluated.
; Results are 1 for true and 0 for false.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 1
; ld r1, 1
; ld r2, 1
; ld r3, 0
; ld r4, 1
; ld r5, 1
; ld r6, 1
; ld r7, 1
; ld r8, 1
; ld r9, 0
; halt
;
;----------------------------------------------------------------------------
