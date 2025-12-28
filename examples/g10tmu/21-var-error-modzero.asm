; Test: Error Case - Modulo by Zero
; This should produce an assembly error.
; Expected error: "Modulo by zero"

.let $x = 100
$x %= 0                 ; ERROR: Modulo by zero
