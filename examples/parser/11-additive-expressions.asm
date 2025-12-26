; Test 11: Additive Expressions
; Tests the parser's ability to parse additive expressions.
; Additive operators: + (addition), - (subtraction)

.org 0x2000

; Simple addition
test_addition:
    ld d0, 1 + 1
    ld d0, 10 + 20
    ld d0, 0xFF + 1
    ld d0, 100 + 200 + 300

; Simple subtraction
test_subtraction:
    ld d0, 10 - 5
    ld d0, 100 - 1
    ld d0, 0x100 - 0x10
    ld d0, 1000 - 100 - 10

; Mixed addition and subtraction (left-associative)
; a + b - c parses as (a + b) - c
test_mixed_additive:
    ld d0, 10 + 5 - 3
    ld d0, 100 - 50 + 25
    ld d0, 1 + 2 + 3 - 4 - 5
    ld d0, 0x100 + 0x10 - 0x1

; Additive with grouped expressions
test_grouped_additive:
    ld d0, (10 + 5)
    ld d0, (100 - 50)
    ld d0, (10 + 5) + (3 + 2)
    ld d0, (100 - 50) - (25 - 10)

; Additive with unary operators
test_additive_unary:
    ld d0, 10 + -5          ; 10 + (-5) = 5
    ld d0, 10 - -5          ; 10 - (-5) = 15
    ld d0, -10 + 5          ; (-10) + 5 = -5
    ld d0, -10 - -5         ; (-10) - (-5) = -5

; Label arithmetic
base_label:
offset_label:
test_label_arithmetic:
    ld d0, base_label + 4
    ld d0, base_label + 0x10
    ld d0, offset_label - base_label

; Data with additive expressions
.org 0x80000000
additive_data:
.byte 10 + 5, 20 - 10
.word 0x100 + 0x50, 0x200 - 0x100
.dword 1000 + 2000, 5000 - 1000
