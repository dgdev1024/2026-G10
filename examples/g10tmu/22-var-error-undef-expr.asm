; Test: Error Case - Undefined Variable in Expression
; This should produce an assembly error.
; Expected error: "Undefined variable"

.let $a = 10
.let $b = $a + $NONEXISTENT     ; ERROR: $NONEXISTENT is not defined
