; Test 5: Braced Expressions in Macro Definitions
; This test verifies that braced expressions are evaluated at define time.

.define EVALUATED_VALUE {1 + 2 * 3}
.define LITERAL_VALUE (1 + 2 * 3)

.org 0x0000

    ; EVALUATED_VALUE should expand to 7 (evaluated at define time)
    ld r0, EVALUATED_VALUE

    ; LITERAL_VALUE should expand to (1 + 2 * 3) (not evaluated)
    ld r1, LITERAL_VALUE

    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Braced Expressions in Macro Definitions
;
; This test verifies that:
; - Expressions in curly braces are evaluated at definition time
; - Expressions without braces are preserved as literal tokens
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 7
; ld r1, (1 + 2 * 3)
; halt
;
;----------------------------------------------------------------------------
