; Test 11: Move Instruction Encoding
; Tests encoding of register-to-register move instructions.

.org 0x2000

; 32-bit Move: MV DX, DY
; Format: 0x3DXY (2 bytes)
test_mv_d:
    mv d0, d0                   ; 0x3D00
    mv d0, d1                   ; 0x3D01
    mv d1, d0                   ; 0x3D10
    mv d5, d10                  ; 0x3D5A
    mv d10, d5                  ; 0x3DA5
    mv d15, d15                 ; 0x3DFF

; 16-bit Move: MV WX, WY
; Format: 0x2DXY (2 bytes)
test_mv_w:
    mv w0, w0                   ; 0x2D00
    mv w0, w1                   ; 0x2D01
    mv w1, w0                   ; 0x2D10
    mv w7, w8                   ; 0x2D78
    mv w15, w15                 ; 0x2DFF

; 8-bit Move L to L: MV LX, LY
; Format: 0x1DXY (2 bytes)
test_mv_l_to_l:
    mv l0, l0                   ; 0x1D00
    mv l0, l1                   ; 0x1D01
    mv l1, l0                   ; 0x1D10
    mv l7, l8                   ; 0x1D78
    mv l15, l15                 ; 0x1DFF

; 8-bit Move L to H: MV HX, LY
; Format: 0x1EXY (2 bytes)
test_mv_l_to_h:
    mv h0, l0                   ; 0x1E00
    mv h0, l1                   ; 0x1E01
    mv h1, l0                   ; 0x1E10
    mv h15, l15                 ; 0x1EFF

; 8-bit Move H to L: MV LX, HY
; Format: 0x1FXY (2 bytes)
test_mv_h_to_l:
    mv l0, h0                   ; 0x1F00
    mv l0, h1                   ; 0x1F01
    mv l1, h0                   ; 0x1F10
    mv l15, h15                 ; 0x1FFF

; Move Word to High: MWH DX, WY
; Format: 0x2EXY (2 bytes)
test_mwh:
    mwh d0, w0                  ; 0x2E00
    mwh d0, w1                  ; 0x2E01
    mwh d1, w0                  ; 0x2E10
    mwh d15, w15                ; 0x2EFF
