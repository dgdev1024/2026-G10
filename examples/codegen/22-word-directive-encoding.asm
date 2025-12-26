; Test 22: Data Directive Encoding (.word)
; Tests encoding of .word directives (16-bit values, little-endian).

.org 0x2000

; Single word values
word_single:
.word 0x0000                    ; Zero: 0x00 0x00
.word 0x0001                    ; One: 0x01 0x00
.word 0x0100                    ; 256: 0x00 0x01
.word 0x7FFF                    ; Max positive signed: 0xFF 0x7F
.word 0x8000                    ; Min negative signed: 0x00 0x80
.word 0xFFFF                    ; Max unsigned: 0xFF 0xFF

; Little-endian byte order verification
word_endian:
.word 0x1234                    ; Should emit: 0x34 0x12
.word 0xABCD                    ; Should emit: 0xCD 0xAB
.word 0xFF00                    ; Should emit: 0x00 0xFF
.word 0x00FF                    ; Should emit: 0xFF 0x00

; Multiple words on one line
word_multiple:
.word 0x0001, 0x0002, 0x0003    ; Sequential
.word 0xDEAD, 0xBEEF            ; Two words
.word 0x1111, 0x2222, 0x3333, 0x4444

; Decimal values
word_decimal:
.word 0, 1, 256, 32767, 65535

; Expression values
word_expressions:
.word 0x100 + 0x200             ; 0x300
.word 0x1000 - 0x100            ; 0x0F00
.word 0x10 * 0x10               ; 0x100
.word 1 << 12                   ; 0x1000
.word 0xFFFF & 0x0F0F           ; 0x0F0F
.word 0xF0F0 | 0x0F0F           ; 0xFFFF
.word 0xAAAA ^ 0x5555           ; 0xFFFF

; Address-sized values (truncated to 16 bits)
word_addresses:
.word 0x2000                    ; Code base
.word 0xFFFF                    ; Quick RAM base (lower word)
