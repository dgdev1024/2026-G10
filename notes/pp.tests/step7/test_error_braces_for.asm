; Test: Braces not allowed in .for expressions
; This test verifies that braces are rejected in .for expressions.

.define START 0
.define END 5

.org 0x0000

; This should produce an error - braces not allowed
.for i, {START}, END
    nop
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Braces not allowed in .for expressions
;
; The preprocessor should reject braces in .for expressions since the entire
; arguments are already expressions.
;
; EXPECTED: ERROR - Curly braces are not allowed in '.for' expressions.
;
;----------------------------------------------------------------------------
