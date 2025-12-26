; Test 05: 32-Bit Load Immediate Encoding
; Tests encoding of 32-bit load immediate instructions.
; Format: 0x30X0 LD DX, IMM32 (6 bytes: opcode + imm32 little-endian)

.org 0x2000

; Test all D registers with immediate values
test_ld_d_imm32:
    ld d0, 0x00000000           ; 0x3000 + 4 bytes
    ld d1, 0x01010101           ; 0x3010 + 4 bytes
    ld d2, 0x02020202           ; 0x3020 + 4 bytes
    ld d3, 0x03030303           ; 0x3030 + 4 bytes
    ld d4, 0x04040404           ; 0x3040 + 4 bytes
    ld d5, 0x05050505           ; 0x3050 + 4 bytes
    ld d6, 0x06060606           ; 0x3060 + 4 bytes
    ld d7, 0x07070707           ; 0x3070 + 4 bytes
    ld d8, 0x08080808           ; 0x3080 + 4 bytes
    ld d9, 0x09090909           ; 0x3090 + 4 bytes
    ld d10, 0x0A0A0A0A          ; 0x30A0 + 4 bytes
    ld d11, 0x0B0B0B0B          ; 0x30B0 + 4 bytes
    ld d12, 0x0C0C0C0C          ; 0x30C0 + 4 bytes
    ld d13, 0x0D0D0D0D          ; 0x30D0 + 4 bytes
    ld d14, 0x0E0E0E0E          ; 0x30E0 + 4 bytes
    ld d15, 0x0F0F0F0F          ; 0x30F0 + 4 bytes

; Test little-endian byte order
test_ld_d_endianness:
    ld d0, 0x12345678           ; Should emit: 0x3000 0x78 0x56 0x34 0x12
    ld d0, 0xDEADBEEF           ; Should emit: 0x3000 0xEF 0xBE 0xAD 0xDE
    ld d0, 0xFF000000           ; High byte set
    ld d0, 0x000000FF           ; Low byte set
    ld d0, 0x00FF0000           ; Third byte set
    ld d0, 0x0000FF00           ; Second byte set

; Test special values
test_ld_d_special:
    ld d0, 0                    ; Zero
    ld d0, 1                    ; One
    ld d0, -1                   ; All bits set (0xFFFFFFFF)
    ld d0, 0x7FFFFFFF           ; Max positive signed
    ld d0, 0x80000000           ; Min negative signed
