; Test 23: Edge Cases and Boundary Conditions
; Tests edge cases in expression parsing and operator handling.

.org 0x2000

; Zero and identity operations
test_identity:
    ld d0, 0 + 0                    ; 0
    ld d0, 0 - 0                    ; 0
    ld d0, 0 * 100                  ; 0
    ld d0, 100 * 0                  ; 0
    ld d0, 0 * 0                    ; 0
    ld d0, 1 * 100                  ; 100 (identity)
    ld d0, 100 * 1                  ; 100 (identity)
    ld d0, 100 + 0                  ; 100 (identity)
    ld d0, 100 - 0                  ; 100 (identity)

; Power of zero and one
test_power_edge:
    ld d0, 0 ** 0                   ; Mathematical edge case
    ld d0, 0 ** 1                   ; 0
    ld d0, 1 ** 0                   ; 1
    ld d0, 1 ** 100                 ; 1
    ld d0, 100 ** 0                 ; 1
    ld d0, 100 ** 1                 ; 100

; Shift edge cases
test_shift_edge:
    ld d0, 0 << 0                   ; 0
    ld d0, 0 << 10                  ; 0
    ld d0, 1 << 0                   ; 1
    ld d0, 0xFF >> 0                ; 0xFF
    ld d0, 0 >> 10                  ; 0

; Bitwise with all zeros and all ones
test_bitwise_edge:
    ld d0, 0 & 0                    ; 0
    ld d0, 0 & 0xFFFFFFFF           ; 0
    ld d0, 0xFFFFFFFF & 0xFFFFFFFF  ; 0xFFFFFFFF
    ld d0, 0 | 0                    ; 0
    ld d0, 0 | 0xFFFFFFFF           ; 0xFFFFFFFF
    ld d0, 0xFFFFFFFF | 0xFFFFFFFF  ; 0xFFFFFFFF
    ld d0, 0 ^ 0                    ; 0
    ld d0, 0xFFFFFFFF ^ 0xFFFFFFFF  ; 0

; Deeply nested parentheses
test_deep_parens:
    ld d0, (1)
    ld d0, ((1))
    ld d0, (((1)))
    ld d0, ((((1))))
    ld d0, (((((1)))))
    ld d0, ((((((1))))))
    ld d0, (((((((1)))))))
    ld d0, ((((((((1))))))))

; Nested expressions at each depth
test_nested_depth:
    ld d0, (1 + (2 + (3 + (4 + (5)))))
    ld d0, ((((1 + 2) + 3) + 4) + 5)
    ld d0, (1 * (2 + (3 * (4 + 5))))

; Multiple consecutive unary operators
test_consecutive_unary:
    ld d0, -1
    ld d0, --1
    ld d0, ---1
    ld d0, ----1
    ld d0, ~0
    ld d0, ~~0
    ld d0, ~~~0
    ld d0, !0
    ld d0, !!0
    ld d0, !!!0

; Mixed consecutive unary operators
test_mixed_consecutive_unary:
    ld d0, -~0
    ld d0, ~-1
    ld d0, -~-1
    ld d0, ~-~0
    ld d0, !-1
    ld d0, -!0
    ld d0, ~!0

; Large numbers
test_large_numbers:
    ld d0, 0xFFFFFFFF
    ld d0, 4294967295
    ld d0, 0x7FFFFFFF
    ld d0, 2147483647

; Hexadecimal with mixed case (if supported)
test_hex_case:
    ld d0, 0xabcdef
    ld d0, 0xABCDEF
    ld d0, 0xAbCdEf
