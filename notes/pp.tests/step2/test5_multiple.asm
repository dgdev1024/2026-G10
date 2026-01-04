; Test 5: Multiple Macros
; This test verifies that multiple macros can be defined and used together.

.define BASE_ADDR 0x1000
.define OFFSET 0x100
.define REG_A r0
.define REG_B r1

.org BASE_ADDR

start:
    ld REG_A, OFFSET
    ld REG_B, BASE_ADDR
    add REG_A, REG_B
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Multiple Macros
;
; This test verifies that multiple macros can be defined and used together
; in the same source file, with each macro being expanded correctly.
;
; EXPECTED OUTPUT:
; .org 0x1000
; start:
; ld r0, 0x100
; ld r1, 0x1000
; add r0, r1
; halt
;
;----------------------------------------------------------------------------
