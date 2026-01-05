; Test 14: .define inside conditional
; This test verifies that .define directives work inside conditionals.

.define BUILD_DEBUG 1

.org 0x0000

.if BUILD_DEBUG
    .define LOG_LEVEL 3
.else
    .define LOG_LEVEL 0
.endif

ld r0, LOG_LEVEL

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .define inside conditional
;
; Macros can be defined inside conditional blocks, and should only be
; defined if the branch is taken.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 3
; halt
;
;----------------------------------------------------------------------------
