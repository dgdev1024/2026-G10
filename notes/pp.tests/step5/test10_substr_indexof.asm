; Test 10: String Functions - substr(), indexof()
; This test verifies substring extraction and search functions.

.define STR "Hello, World!"

.org 0x0000

.byte {indexof(STR, "World")}   ; Index of "World" in STR = 7
.byte {indexof(STR, "xyz")}     ; Not found = -1
.byte {indexof(STR, "o")}       ; First 'o' at index 4

.string {substr(STR, 0, 5)}     ; Should be "Hello"
.string {substr(STR, 7, 5)}     ; Should be "World"
.string {substr(STR, 7)}        ; Should be "World!"

halt

;----------------------------------------------------------------------------
;
; TEST CASE: String functions substr() and indexof()
;
; This test verifies:
; - indexof() returns the position of a substring, or -1 if not found
; - substr() extracts a substring with start and optional length
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 7
; .byte -1
; .byte 4
; .string "Hello"
; .string "World"
; .string "World!"
; halt
;
;----------------------------------------------------------------------------
