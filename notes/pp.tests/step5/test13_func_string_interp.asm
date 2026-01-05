; Test 13: Functions in String Interpolation
; This test verifies that built-in functions work inside string interpolation
; and that string-returning functions do NOT include quotes in the result.

.define NAME "World"
.define VALUE 42

.org 0x0000

; Numeric functions in strings
.string "The value is {abs(-25)}."                  ; Should be: "The value is 25."
.string "Max of 10 and 20 is {max(10, 20)}."        ; Should be: "Max of 10 and 20 is 20."
.string "Clamped: {clamp(50, 0, 30)}."              ; Should be: "Clamped: 30."

; String functions in strings (no quotes around result)
.string "Upper: {toupper(NAME)}"                    ; Should be: "Upper: WORLD"
.string "Sub: {substr(NAME, 0, 3)}"                 ; Should be: "Sub: Wor"
.string "Len: {strlen(NAME)}"                       ; Should be: "Len: 5"

; Multiple functions in one string
.string "{toupper(NAME)} has {strlen(NAME)} chars"  ; Should be: "WORLD has 5 chars"

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Built-in functions in string interpolation
;
; This test verifies that:
; - Numeric functions work inside string interpolation
; - String-returning functions work and do NOT include quotes
; - Multiple function calls can be used in a single string
;
; EXPECTED OUTPUT:
; .org 0x0000
; .string "The value is 25."
; .string "Max of 10 and 20 is 20."
; .string "Clamped: 30."
; .string "Upper: WORLD"
; .string "Sub: Wor"
; .string "Len: 5"
; .string "WORLD has 5 chars"
; halt
;
;----------------------------------------------------------------------------
