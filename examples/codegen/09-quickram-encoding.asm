; Test 09: Quick RAM Load/Store Encoding
; Tests encoding of quick RAM operations (16-bit relative addressing).
; Quick RAM addresses are relative to 0xFFFF0000.

.org 0x2000

; LDQ - Load from Quick RAM (direct)
; Format: 0x33X0 LDQ DX, [ADDR16] (4 bytes)
test_ldq_direct:
    ldq d0, [0x0000]            ; Address = 0xFFFF0000
    ldq d0, [0x0100]            ; Address = 0xFFFF0100
    ldq d0, [0x1234]            ; Address = 0xFFFF1234
    ldq d0, [0xFFFF]            ; Address = 0xFFFFFFFF
    ldq d5, [0x0500]
    ldq d15, [0x0F00]

; LDQ - Load from Quick RAM (indirect via W register)
; Format: 0x34XY LDQ DX, [WY] (2 bytes)
test_ldq_indirect:
    ldq d0, [w0]                ; 0x3400
    ldq d0, [w1]                ; 0x3401
    ldq d1, [w0]                ; 0x3410
    ldq d15, [w15]              ; 0x34FF

; STQ - Store to Quick RAM (direct)
; Format: 0x390Y STQ [ADDR16], DY (4 bytes)
test_stq_direct:
    stq [0x0000], d0            ; 0x3900 + addr16
    stq [0x0100], d1            ; 0x3901 + addr16
    stq [0x1234], d7            ; 0x3907 + addr16
    stq [0xFFFF], d15           ; 0x390F + addr16

; STQ - Store to Quick RAM (indirect via W register)
; Format: 0x3AXY STQ [WX], DY (2 bytes)
test_stq_indirect:
    stq [w0], d0                ; 0x3A00
    stq [w0], d1                ; 0x3A01
    stq [w1], d0                ; 0x3A10
    stq [w15], d15              ; 0x3AFF
