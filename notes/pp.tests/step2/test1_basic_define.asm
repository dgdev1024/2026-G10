; Test 1: Basic Macro Definition and Expansion
; This test verifies that basic text-substitution macros work correctly.

.define VALUE 42
.define REG r0

.org 0x0000

start:
    ld REG, VALUE
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Basic Macro Definition and Expansion
;
; This test verifies that the preprocessor correctly defines and expands
; simple text-substitution macros. The macro VALUE should be replaced with
; 42, and REG should be replaced with r0.
;
; EXPECTED OUTPUT:
; .org 0x0000
; start:
; ld r0, 42
; halt
;
;----------------------------------------------------------------------------
