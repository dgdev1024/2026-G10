; Test: Error Case - Duplicate Definition
; This should produce an assembly error.
; Expected error: "already defined"

.let $MY_VAR = 10
.let $MY_VAR = 20       ; ERROR: Already defined
