; Test: Error Case - Division by Zero
; This should produce an assembly error.
; Expected error: "Division by zero"

.let $x = 100
$x /= 0                 ; ERROR: Division by zero
