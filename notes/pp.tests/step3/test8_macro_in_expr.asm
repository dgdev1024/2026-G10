; Test 8: Macro Reference in Expression
; This test verifies that defined macros can be used within expressions.

.define BASE 100
.define OFFSET 50
.define SUM {BASE + OFFSET}
.define SUM {SUM * 2}

.org 0x0000

    ld r0, {BASE + OFFSET}       ; Should evaluate to 150
    ld r1, {BASE * 2}            ; Should evaluate to 200
    ld r2, {BASE + OFFSET * 2}   ; Should evaluate to 200 (precedence)
    ld r3, {SUM / 6}             ; Should evaluate to 50
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Macro Reference in Expression
;
; This test verifies that macros defined with simple values can be
; referenced and evaluated within braced expressions.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 150
; ld r1, 200
; ld r2, 200
; ld r3, 50
; halt
;
;----------------------------------------------------------------------------
