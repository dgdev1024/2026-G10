; Test 11: String Functions - toupper(), tolower(), concat()
; This test verifies case conversion and concatenation functions.

.define LOWER "hello"
.define UPPER "WORLD"

.org 0x0000

.string {toupper(LOWER)}        ; Should be "HELLO"
.string {tolower(UPPER)}        ; Should be "world"
.string {concat(LOWER, " ", UPPER)} ; Should be "hello WORLD"

halt

;----------------------------------------------------------------------------
;
; TEST CASE: String functions toupper(), tolower(), concat()
;
; This test verifies:
; - toupper() converts to uppercase
; - tolower() converts to lowercase
; - concat() concatenates multiple strings
;
; EXPECTED OUTPUT:
; .org 0x0000
; .string "HELLO"
; .string "world"
; .string "hello WORLD"
; halt
;
;----------------------------------------------------------------------------
