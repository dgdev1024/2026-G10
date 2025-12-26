;
; 4-registers.asm
; Tests all register names.
;

.org 0x1000
test_registers:
    ld d0, 0
    ld d1, 1
    ld d2, 2
    ld d3, 3
    ld d4, 4
    ld d5, 5
    ld d6, 6
    ld d7, 7
    ld d8, 8
    ld d9, 9
    ld d10, 10
    ld d11, 11
    ld d12, 12
    ld d13, 13
    ld d14, 14
    ld d15, 15

    ld w0, 0
    ld w1, 1
    ld w2, 2
    ld w3, 3
    ld w4, 4
    ld w5, 5
    ld w6, 6
    ld w7, 7
    ld w8, 8
    ld w9, 9
    ld w10, 10
    ld w11, 11
    ld w12, 12
    ld w13, 13
    ld w14, 14
    ld w15, 15

    ld h0, 0
    ld h1, 1
    ld h2, 2
    ld h3, 3
    ld h4, 4
    ld h5, 5
    ld h6, 6
    ld h7, 7
    ld h8, 8
    ld h9, 9
    ld h10, 10
    ld h11, 11
    ld h12, 12
    ld h13, 13
    ld h14, 14
    ld h15, 15

    ld l0, 0
    ld l1, 1
    ld l2, 2
    ld l3, 3
    ld l4, 4
    ld l5, 5
    ld l6, 6
    ld l7, 7
    ld l8, 8
    ld l9, 9
    ld l10, 10
    ld l11, 11
    ld l12, 12
    ld l13, 13
    ld l14, 14
    ld l15, 15