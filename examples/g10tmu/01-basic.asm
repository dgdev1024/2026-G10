.global main

.org 0x80000000
    age_bcd:    .byte 1
    test:       .word 1

.org 0x2000
main:
    ld l0, ((0x21 * 4) / 2)
    st [age_bcd], l0
    ld w1, 0x5678
    st [test], w1
    stop
