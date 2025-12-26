; Test 21: Data Directive Encoding (.byte)
; Tests encoding of .byte directives with various values.

.org 0x2000

; Single byte values
byte_single:
.byte 0x00                      ; Zero
.byte 0x01                      ; One
.byte 0x7F                      ; Max positive signed
.byte 0x80                      ; Min negative signed
.byte 0xFF                      ; Max unsigned / -1

; Multiple bytes on one line
byte_multiple:
.byte 0x01, 0x02, 0x03, 0x04    ; Sequential bytes
.byte 0xDE, 0xAD, 0xBE, 0xEF    ; 0xDEADBEEF in bytes (big-endian order)
.byte 0x00, 0xFF, 0x00, 0xFF    ; Alternating pattern

; Decimal values
byte_decimal:
.byte 0, 1, 127, 128, 255       ; Decimal values

; Binary values
byte_binary:
.byte 0b00000001                ; 1
.byte 0b00001111                ; 15
.byte 0b11110000                ; 240
.byte 0b10101010                ; 170

; Character values
byte_chars:
.byte 'A', 'B', 'C', 'D'        ; ASCII characters
.byte 'a', 'b', 'c', 'd'
.byte '0', '1', '2', '3'
.byte ' ', '\n', '\r', '\t'     ; Whitespace characters

; Expression values
byte_expressions:
.byte 1 + 1                     ; 2
.byte 10 - 5                    ; 5
.byte 4 * 3                     ; 12
.byte 100 / 10                  ; 10
.byte 17 % 5                    ; 2
.byte 2 ** 4                    ; 16
.byte 1 << 4                    ; 16
.byte 0x80 >> 4                 ; 8
.byte 0xFF & 0x0F               ; 15
.byte 0xF0 | 0x0F               ; 255
.byte 0xAA ^ 0x55               ; 255
