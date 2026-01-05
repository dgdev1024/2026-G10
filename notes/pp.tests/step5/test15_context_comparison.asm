; Test 15: Mixed Functions - Standalone vs Interpolation Contexts
; This test verifies that string-returning functions behave correctly:
; - In standalone braced expressions: strings ARE quoted
; - In string interpolation: strings are NOT quoted
; - In identifier interpolation: strings are NOT quoted

.define STR "hello"

.org 0x0000

; Standalone braced expression - should have quotes
.string {toupper(STR)}          ; Should be: .string "HELLO"

; String interpolation - no quotes around the function result
.string "Say: {toupper(STR)}"   ; Should be: .string "Say: HELLO"

; Identifier interpolation - no quotes
{toupper(STR)}_label:           ; Should be: HELLO_label:
    nop

; Compare typeof in different contexts
.string {typeof(42)}            ; Should be: .string "integer"
.string "Type: {typeof(42)}"    ; Should be: .string "Type: integer"

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Standalone vs interpolation contexts for string functions
;
; This test verifies that:
; - Standalone {func()} adds quotes to string results
; - String interpolation "{func()}" does NOT add quotes
; - Identifier {func()}_suffix does NOT add quotes
;
; EXPECTED OUTPUT:
; .org 0x0000
; .string "HELLO"
; .string "Say: HELLO"
; HELLO_label:
; nop
; .string "integer"
; .string "Type: integer"
; halt
;
;----------------------------------------------------------------------------
