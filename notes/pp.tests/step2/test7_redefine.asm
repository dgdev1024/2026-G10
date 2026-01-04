; Test 7: Macro Redefinition
; This test verifies that macros can be redefined (replacing old definition).

.define VALUE 10
    ld r0, VALUE        ; Should use VALUE = 10

.define VALUE 20
    ld r1, VALUE        ; Should use VALUE = 20

.define VALUE 30
    ld r2, VALUE        ; Should use VALUE = 30

    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Macro Redefinition
;
; This test verifies that macros can be redefined without first using .undef.
; Each redefinition should replace the previous definition.
;
; EXPECTED OUTPUT:
; ld r0, 10
; ld r1, 20
; ld r2, 30
; halt
;
;----------------------------------------------------------------------------
