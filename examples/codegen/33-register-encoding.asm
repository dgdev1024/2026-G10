; Test 33: Register Index Encoding
; Tests that register indices (0-15) are correctly encoded in opcodes.

.org 0x2000

; All 16 D registers in INC instruction (0x6DX0)
test_d_registers:
    inc d0                      ; 0x6D00
    inc d1                      ; 0x6D10
    inc d2                      ; 0x6D20
    inc d3                      ; 0x6D30
    inc d4                      ; 0x6D40
    inc d5                      ; 0x6D50
    inc d6                      ; 0x6D60
    inc d7                      ; 0x6D70
    inc d8                      ; 0x6D80
    inc d9                      ; 0x6D90
    inc d10                     ; 0x6DA0
    inc d11                     ; 0x6DB0
    inc d12                     ; 0x6DC0
    inc d13                     ; 0x6DD0
    inc d14                     ; 0x6DE0
    inc d15                     ; 0x6DF0

; All 16 W registers in INC instruction (0x6CX0)
test_w_registers:
    inc w0                      ; 0x6C00
    inc w1                      ; 0x6C10
    inc w2                      ; 0x6C20
    inc w3                      ; 0x6C30
    inc w4                      ; 0x6C40
    inc w5                      ; 0x6C50
    inc w6                      ; 0x6C60
    inc w7                      ; 0x6C70
    inc w8                      ; 0x6C80
    inc w9                      ; 0x6C90
    inc w10                     ; 0x6CA0
    inc w11                     ; 0x6CB0
    inc w12                     ; 0x6CC0
    inc w13                     ; 0x6CD0
    inc w14                     ; 0x6CE0
    inc w15                     ; 0x6CF0

; All 16 L registers in INC instruction (0x5CX0)
test_l_registers:
    inc l0                      ; 0x5C00
    inc l1                      ; 0x5C10
    inc l2                      ; 0x5C20
    inc l3                      ; 0x5C30
    inc l4                      ; 0x5C40
    inc l5                      ; 0x5C50
    inc l6                      ; 0x5C60
    inc l7                      ; 0x5C70
    inc l8                      ; 0x5C80
    inc l9                      ; 0x5C90
    inc l10                     ; 0x5CA0
    inc l11                     ; 0x5CB0
    inc l12                     ; 0x5CC0
    inc l13                     ; 0x5CD0
    inc l14                     ; 0x5CE0
    inc l15                     ; 0x5CF0

; Two-register instructions: MV DX, DY (0x3DXY)
test_two_reg_encoding:
    mv d0, d0                   ; 0x3D00 (X=0, Y=0)
    mv d0, d15                  ; 0x3D0F (X=0, Y=15)
    mv d15, d0                  ; 0x3DF0 (X=15, Y=0)
    mv d15, d15                 ; 0x3DFF (X=15, Y=15)
    mv d5, d10                  ; 0x3D5A (X=5, Y=10)
    mv d10, d5                  ; 0x3DA5 (X=10, Y=5)
    mv d7, d8                   ; 0x3D78 (X=7, Y=8)
    mv d8, d7                   ; 0x3D87 (X=8, Y=7)

; Store with register index (0x370Y for source)
test_store_reg_index:
    st [0x80000000], d0         ; Source = d0
    st [0x80000000], d1         ; Source = d1
    st [0x80000000], d7         ; Source = d7
    st [0x80000000], d15        ; Source = d15

; Indirect addressing with register index
test_indirect_reg_index:
    ld d0, [d0]                 ; Address in d0
    ld d0, [d1]                 ; Address in d1
    ld d0, [d7]                 ; Address in d7
    ld d0, [d15]                ; Address in d15
