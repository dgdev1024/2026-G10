; Test: Braces not allowed in .while condition
; This test verifies that braces are rejected in .while conditions.

.define LIMIT 3

.org 0x0000

; This should produce an error - braces not allowed
.while {LIMIT > 0}
    nop
    .define LIMIT {LIMIT - 1}
.endwhile

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Braces not allowed in .while condition
;
; The preprocessor should reject braces in .while conditions since the entire
; argument is already an expression.
;
; EXPECTED: ERROR - Curly braces are not allowed in '.while' expressions.
;
;----------------------------------------------------------------------------
