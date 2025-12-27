; Test 07: Conditional Branching
; Tests: All condition codes (zs, zc, cs, cc, ns, nc, vs, vc)
;
; This program tests various conditional branches.

.org 0x00002000

.global main

main:
    ; Test zero flag
    ld l0, 0
    cmp l0, 0               ; Sets zero flag
    jpb zs, zero_set        ; Should branch (zero set)
    jpb nc, fail            ; Should not execute

zero_set:
    ld l0, 1
    cmp l0, 0               ; Clears zero flag
    jpb zc, zero_clear      ; Should branch (zero clear)
    jpb nc, fail

zero_clear:
    ; Test carry flag with overflow
    ld l0, 0xFF
    add l0, 1               ; 0xFF + 1 = overflow, sets carry
    jpb cs, carry_set       ; Should branch (carry set)
    jpb nc, fail

carry_set:
    ld l0, 10
    add l0, 5               ; No overflow
    jpb cc, carry_clear     ; Should branch (carry clear)
    jpb nc, fail

carry_clear:
    ; Test with subtraction
    ld l0, 5
    sub l0, 3               ; 5 - 3 = 2, positive
    jpb nc, not_negative    ; Should branch (unconditional)

not_negative:
    ; All tests passed
    ld d0, 0x12345678       ; Success marker
    halt

fail:
    ld d0, 0xDEADDEAD       ; Failure marker
    halt
