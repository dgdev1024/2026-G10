; Test 23: Data Directive Encoding (.dword)
; Tests encoding of .dword directives (32-bit values, little-endian).

.org 0x2000

; Single dword values
dword_single:
.dword 0x00000000               ; Zero: 0x00 0x00 0x00 0x00
.dword 0x00000001               ; One: 0x01 0x00 0x00 0x00
.dword 0x7FFFFFFF               ; Max positive signed
.dword 0x80000000               ; Min negative signed
.dword 0xFFFFFFFF               ; Max unsigned / -1

; Little-endian byte order verification
dword_endian:
.dword 0x12345678               ; Should emit: 0x78 0x56 0x34 0x12
.dword 0xDEADBEEF               ; Should emit: 0xEF 0xBE 0xAD 0xDE
.dword 0xFF000000               ; High byte set
.dword 0x000000FF               ; Low byte set
.dword 0x00FF0000               ; Third byte set
.dword 0x0000FF00               ; Second byte set

; Multiple dwords on one line
dword_multiple:
.dword 0x00000001, 0x00000002   ; Two values
.dword 0xDEADBEEF, 0xCAFEBABE, 0x12345678

; Decimal values
dword_decimal:
.dword 0, 1, 256, 65536, 2147483647

; Expression values
dword_expressions:
.dword 0x10000 + 0x20000        ; 0x30000
.dword 0x100000 - 0x10000       ; 0xF0000
.dword 0x100 * 0x100            ; 0x10000
.dword 1 << 20                  ; 0x100000
.dword 0xFFFFFFFF & 0x0F0F0F0F  ; 0x0F0F0F0F
.dword 0xF0F0F0F0 | 0x0F0F0F0F  ; 0xFFFFFFFF
.dword 0xAAAAAAAA ^ 0x55555555  ; 0xFFFFFFFF

; Power of 2 values
dword_powers:
.dword 2 ** 0                   ; 1
.dword 2 ** 8                   ; 256
.dword 2 ** 16                  ; 65536
.dword 2 ** 24                  ; 16777216
.dword 2 ** 31                  ; 2147483648

; Address values
dword_addresses:
.dword 0x00002000               ; ROM base
.dword 0x80000000               ; RAM base
.dword 0xFFFF0000               ; Quick RAM base
.dword 0xFFFFFF00               ; I/O port base
