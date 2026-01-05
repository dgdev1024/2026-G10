; Test 9: String Functions - strlen(), strcmp()
; This test verifies string length and comparison functions.

.define STR1 "Hello"
.define STR2 "World"
.define STR3 "Hello"

.org 0x0000

.byte {strlen(STR1)}            ; Length of "Hello" = 5
.byte {strlen(STR2)}            ; Length of "World" = 5
.byte {strlen("")}              ; Length of "" = 0
.byte {strcmp(STR1, STR2)}      ; "Hello" < "World" -> -1
.byte {strcmp(STR2, STR1)}      ; "World" > "Hello" -> 1
.byte {strcmp(STR1, STR3)}      ; "Hello" == "Hello" -> 0

halt

;----------------------------------------------------------------------------
;
; TEST CASE: String functions strlen() and strcmp()
;
; This test verifies:
; - strlen() returns the length of a string
; - strcmp() compares strings lexicographically
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 5
; .byte 5
; .byte 0
; .byte -1
; .byte 1
; .byte 0
; halt
;
;----------------------------------------------------------------------------
