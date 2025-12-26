; Test 06: Direct Memory Load Encoding
; Tests encoding of load instructions with direct memory addressing.
; Format: 0x31X0 LD DX, [ADDR32] (6 bytes: opcode + addr32)

.org 0x2000

; Test loading from various addresses
test_ld_direct:
    ld d0, [0x80000000]         ; 0x3100 + address
    ld d1, [0x80000004]         ; 0x3110 + address
    ld d2, [0x80000008]         ; 0x3120 + address
    ld d15, [0xFFFFFFFC]        ; 0x31F0 + address

; Test address byte order (little-endian)
test_ld_direct_endian:
    ld d0, [0x12345678]         ; Address should be 0x78 0x56 0x34 0x12
    ld d0, [0xDEADBEEF]         ; Address should be 0xEF 0xBE 0xAD 0xDE

; 8-bit load from direct address
test_ld8_direct:
    ld l0, [0x80000000]         ; 0x1200 + address
    ld l5, [0x80000010]         ; 0x1250 + address
    ld h0, [0x80000020]         ; 0x1300 + address

; 16-bit load from direct address
test_ld16_direct:
    ld w0, [0x80000000]         ; 0x2100 + address
    ld w7, [0x80000100]         ; 0x2170 + address
