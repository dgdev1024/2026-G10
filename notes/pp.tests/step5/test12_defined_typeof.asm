; Test 12: Miscellaneous Functions - defined(), typeof()
; This test verifies the defined() and typeof() functions.

.define EXISTING_MACRO 42

.org 0x0000

.byte {defined(EXISTING_MACRO)}     ; Should be 1 (defined)
.byte {defined(NOT_DEFINED)}        ; Should be 0 (not defined)

.string {typeof(42)}                ; Should be "integer"
.string {typeof(3.14)}              ; Should be "fixed-point"
.string {typeof("hello")}           ; Should be "string"

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Miscellaneous functions defined() and typeof()
;
; This test verifies:
; - defined() returns 1 if a macro is defined, 0 otherwise
; - typeof() returns the type name of an expression
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 1
; .byte 0
; .string "integer"
; .string "fixed-point"
; .string "string"
; halt
;
;----------------------------------------------------------------------------
