;
; @file     examples/1-counter.asm
; @author   Dennis W. Griffin <dgdev1024@gmail.com>
; @date     2025-12-21
;
; @brief    Example program that implements a simple counter, using RAM
;           to store the current count value.
;

.global main

.org 0x80000000
counter:
    .byte 1

.org 0x2000
target:
    .byte 0x80

main:
    ld l0, 0
    ld l1, [target]
    st [counter], l0

loop:
    ld l0, [counter]
    inc l0
    cmp l0, l1
    jpb zc, loop

done:
    jpb nc, done
    