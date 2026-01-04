; Test 3: Macro Undefined with .undef
; This test verifies that macros can be undefined using .undef.

.define VALUE 100

.org 0x0000

    ld r0, VALUE        ; VALUE is 100 here

.undef VALUE
.define VALUE 200

    ld r1, VALUE        ; VALUE is 200 now

    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Macro Undefined with .undef
;
; This test verifies that the .undef directive correctly removes a macro
; definition, allowing it to be redefined with a different value.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 100
; ld r1, 200
; halt
;
;----------------------------------------------------------------------------
