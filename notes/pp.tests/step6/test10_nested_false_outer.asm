; Test 10: Nested conditionals with false outer
; This test verifies that nested conditionals properly skip when parent is false.

.define OUTER 0
.define INNER 1

.org 0x0000

.if OUTER
    ld r0, 1        ; Should NOT appear
    .if INNER
        ld r1, 2    ; Should NOT appear (parent is false)
    .endif
    ld r2, 3        ; Should NOT appear
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Nested conditionals with false outer
;
; When OUTER is false, all nested code should be excluded regardless of
; INNER's value.
;
; EXPECTED OUTPUT:
; .org 0x0000
; halt
;
;----------------------------------------------------------------------------
