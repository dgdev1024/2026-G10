; Test 14: Functions in Identifier (Label) Interpolation
; This test verifies that built-in functions work in identifier interpolation
; and that string-returning functions do NOT include quotes in the result.

.define PREFIX "data"
.define SUFFIX "reg"
.define INDEX 5

.org 0x0000

; Numeric functions in identifiers
label_{abs(-10)}:               ; Should be: label_10:
    nop

item_{max(3, 7)}:               ; Should be: item_7:
    nop

; String functions in identifiers (no quotes)
{toupper(PREFIX)}_start:        ; Should be: DATA_start:
    nop

{tolower(SUFFIX)}_{INDEX}:      ; Should be: reg_5:
    nop

; Substr in identifier
{substr(PREFIX, 0, 3)}_ptr:     ; Should be: dat_ptr:
    nop

; Concat in identifier - need suffix or prefix for identifier pattern
{concat(PREFIX, SUFFIX)}_label: ; Should be: datareg_label:
    nop

; Identifier enterly from interpolation
{concat("data", "reg")}:        ; Should be: datareg:
    nop

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Built-in functions in identifier interpolation
;
; This test verifies that:
; - Numeric functions work in identifier interpolation
; - String-returning functions work and do NOT include quotes
; - The resulting identifiers are valid labels
;
; EXPECTED OUTPUT:
; .org 0x0000
; label_10:
; nop
; item_7:
; nop
; DATA_start:
; nop
; reg_5:
; nop
; dat_ptr:
; nop
; datareg_label:
; nop
; datareg:
; nop
; halt
;
;----------------------------------------------------------------------------
