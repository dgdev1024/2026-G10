; Test: Braces not allowed in .if condition
; This test verifies that braces are rejected in .if conditions.

.define VALUE 5

.org 0x0000

; This should produce an error - braces not allowed
.if {VALUE > 3}
    nop
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Braces not allowed in .if condition
;
; The preprocessor should reject braces in .if conditions since the entire
; argument is already an expression.
;
; EXPECTED: ERROR - Braces '{}' are not allowed in '.if/.elif' expressions.
;
;----------------------------------------------------------------------------
