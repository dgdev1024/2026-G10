; Test: Error Case - Modifying a Constant
; This should produce an assembly error.
; Expected error: "Cannot modify constant"

.const $MY_CONST = 42
$MY_CONST = 100         ; ERROR: Cannot modify constant
