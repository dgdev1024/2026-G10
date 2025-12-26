; Test 10: Unary Expressions
; Tests the parser's ability to parse unary expressions.
; Unary operators: - (negation), ~ (bitwise NOT), ! (logical NOT)

.org 0x2000

; Arithmetic negation (-)
test_negation:
    ld d0, -1
    ld d0, -42
    ld d0, -0xFF
    ld d0, -255

; Bitwise NOT (~)
test_bitwise_not:
    ld d0, ~0
    ld d0, ~1
    ld d0, ~0xFF
    ld d0, ~0xFFFFFFFF

; Logical NOT (!)
test_logical_not:
    ld d0, !0
    ld d0, !1
    ld d0, !42

; Nested unary operators
test_nested_unary:
    ld d0, --1              ; Double negation: -(-1) = 1
    ld d0, ~~0xFF           ; Double bitwise NOT: ~(~0xFF) = 0xFF
    ld d0, !!0              ; Double logical NOT: !(!0) = 0
    ld d0, !!1              ; Double logical NOT: !(!1) = 1
    ld d0, ---5             ; Triple negation

; Mixed unary operators
test_mixed_unary:
    ld d0, -~0              ; Negate the bitwise NOT of 0
    ld d0, ~-1              ; Bitwise NOT of -1
    ld d0, !-1              ; Logical NOT of -1

; Unary with grouped expressions
test_unary_grouped:
    ld d0, -(42)
    ld d0, ~(0xFF)
    ld d0, !(0)
    ld d0, -(-(-1))

; Unary with label references
some_label:
test_unary_labels:
    ld d0, -some_label      ; Negate label address
    ld d0, ~some_label      ; Bitwise NOT of label address

; Data directives with unary expressions (ROM region for actual data values)
.org 0x2200
unary_data:
.byte -1, ~0, !0
.word -0x100, ~0xFF00
.dword -1, ~0, -0xDEADBEEF

; Reservation in RAM region (BSS - counts only, no negative counts allowed)
.org 0x80000000
reserved_data:
.byte 10                    ; Reserve 10 bytes
