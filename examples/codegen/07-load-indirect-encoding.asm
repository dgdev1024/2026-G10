; Test 07: Indirect Memory Load Encoding
; Tests encoding of load instructions with indirect (register) addressing.
; Format: 0x32XY LD DX, [DY] (2 bytes: opcode only)

.org 0x2000

; Test all source register combinations for D0
test_ld_indirect_d0:
    ld d0, [d0]                 ; 0x3200
    ld d0, [d1]                 ; 0x3201
    ld d0, [d2]                 ; 0x3202
    ld d0, [d3]                 ; 0x3203
    ld d0, [d7]                 ; 0x3207
    ld d0, [d15]                ; 0x320F

; Test various destination registers
test_ld_indirect_dest:
    ld d1, [d0]                 ; 0x3210
    ld d2, [d0]                 ; 0x3220
    ld d7, [d0]                 ; 0x3270
    ld d15, [d0]                ; 0x32F0

; Test mixed combinations
test_ld_indirect_mixed:
    ld d1, [d2]                 ; 0x3212
    ld d5, [d10]                ; 0x325A
    ld d10, [d5]                ; 0x32A5
    ld d15, [d15]               ; 0x32FF

; 8-bit indirect load
test_ld8_indirect:
    ld l0, [d0]                 ; 0x140Y format
    ld l0, [d1]
    ld l0, [d15]

; 16-bit indirect load
test_ld16_indirect:
    ld w0, [d0]                 ; 0x220Y format
    ld w0, [d7]
    ld w0, [d15]
