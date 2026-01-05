; Test: Braces not allowed in .repeat count
; This test verifies that braces are rejected in .repeat count expressions.

.define COUNT 3

.org 0x0000

; This should produce an error - braces not allowed
.repeat {COUNT}
    nop
.endrepeat

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Braces not allowed in .repeat count
;
; The preprocessor should reject braces in .repeat count since the entire
; argument is already an expression.
;
; EXPECTED: ERROR - Curly braces are not allowed in '.repeat/.rept' expressions.
;
;----------------------------------------------------------------------------
