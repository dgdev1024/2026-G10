; Test 10: I/O Port Load/Store Encoding
; Tests encoding of I/O port operations (8-bit relative addressing).
; I/O port addresses are relative to 0xFFFFFF00.

.org 0x2000

; LDP - Load from I/O Port (direct)
; Format: 0x15X0 LDP LX, [ADDR8] (3 bytes)
test_ldp_direct:
    ldp l0, [0x00]              ; Port 0x00 (addr = 0xFFFFFF00)
    ldp l0, [0x10]              ; Port 0x10
    ldp l0, [0x7F]              ; Port 0x7F
    ldp l0, [0xFF]              ; Port 0xFF (addr = 0xFFFFFFFF)
    ldp l5, [0x50]
    ldp l15, [0xF0]

; LDP - Load from I/O Port (indirect via L register)
; Format: 0x16XY LDP LX, [LY] (2 bytes)
test_ldp_indirect:
    ldp l0, [l0]                ; 0x1600
    ldp l0, [l1]                ; 0x1601
    ldp l1, [l0]                ; 0x1610
    ldp l15, [l15]              ; 0x16FF

; STP - Store to I/O Port (direct)
; Format: 0x1B0Y STP [ADDR8], LY (3 bytes)
test_stp_direct:
    stp [0x00], l0              ; 0x1B00 + port
    stp [0x10], l1              ; 0x1B01 + port
    stp [0x7F], l7              ; 0x1B07 + port
    stp [0xFF], l15             ; 0x1B0F + port

; STP - Store to I/O Port (indirect via L register)
; Format: 0x1CXY STP [LX], LY (2 bytes)
test_stp_indirect:
    stp [l0], l0                ; 0x1C00
    stp [l0], l1                ; 0x1C01
    stp [l1], l0                ; 0x1C10
    stp [l15], l15              ; 0x1CFF
