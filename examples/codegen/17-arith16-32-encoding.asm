; Test 17: 16-Bit and 32-Bit Arithmetic Encoding
; Tests encoding of 16-bit and 32-bit arithmetic instructions.

.org 0x2000

; 16-bit ADD W0, IMM16 - 0x6000
test_add_w_imm:
    add w0, 0x0000              ; 0x6000 0x00 0x00
    add w0, 0x1234              ; 0x6000 0x34 0x12
    add w0, 0xFFFF              ; 0x6000 0xFF 0xFF

; 16-bit ADD W0, WY - 0x610Y
test_add_w_reg:
    add w0, w0                  ; 0x6100
    add w0, w1                  ; 0x6101
    add w0, w7                  ; 0x6107
    add w0, w15                 ; 0x610F

; 32-bit ADD D0, IMM32 - 0x6200
test_add_d_imm:
    add d0, 0x00000000          ; 0x6200 + 4 bytes
    add d0, 0x12345678          ; 0x6200 + 0x78 0x56 0x34 0x12
    add d0, 0xFFFFFFFF          ; 0x6200 + 0xFF 0xFF 0xFF 0xFF

; 32-bit ADD D0, DY - 0x630Y
test_add_d_reg:
    add d0, d0                  ; 0x6300
    add d0, d1                  ; 0x6301
    add d0, d7                  ; 0x6307
    add d0, d15                 ; 0x630F

; 16-bit SUB W0, IMM16 - 0x6400
test_sub_w_imm:
    sub w0, 0x0000              ; 0x6400 0x00 0x00
    sub w0, 0x1234              ; 0x6400 0x34 0x12
    sub w0, 0xFFFF              ; 0x6400 0xFF 0xFF

; 16-bit SUB W0, WY - 0x650Y
test_sub_w_reg:
    sub w0, w0                  ; 0x6500
    sub w0, w1                  ; 0x6501
    sub w0, w15                 ; 0x650F

; 32-bit SUB D0, IMM32 - 0x6600
test_sub_d_imm:
    sub d0, 0x00000000          ; 0x6600 + 4 bytes
    sub d0, 0x12345678          ; 0x6600 + 4 bytes
    sub d0, 0xFFFFFFFF          ; 0x6600 + 4 bytes

; 32-bit SUB D0, DY - 0x670Y
test_sub_d_reg:
    sub d0, d0                  ; 0x6700
    sub d0, d1                  ; 0x6701
    sub d0, d15                 ; 0x670F

; INC WX - 0x6CX0
test_inc_w:
    inc w0                      ; 0x6C00
    inc w1                      ; 0x6C10
    inc w7                      ; 0x6C70
    inc w15                     ; 0x6CF0

; INC DX - 0x6DX0
test_inc_d:
    inc d0                      ; 0x6D00
    inc d1                      ; 0x6D10
    inc d7                      ; 0x6D70
    inc d15                     ; 0x6DF0

; DEC WX - 0x6EX0
test_dec_w:
    dec w0                      ; 0x6E00
    dec w1                      ; 0x6E10
    dec w7                      ; 0x6E70
    dec w15                     ; 0x6EF0

; DEC DX - 0x6FX0
test_dec_d:
    dec d0                      ; 0x6F00
    dec d1                      ; 0x6F10
    dec d7                      ; 0x6F70
    dec d15                     ; 0x6FF0
