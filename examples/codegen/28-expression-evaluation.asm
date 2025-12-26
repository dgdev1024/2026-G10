; Test 28: Expression Evaluation in Code Generation
; Tests that complex expressions are correctly evaluated at assemble time.

.org 0x2000

; Constant definitions via labels
BUFFER_SIZE:
.dword 1024
PAGE_SIZE:
.dword 4096
MAX_VALUE:
.dword 0x7FFFFFFF

; Arithmetic expressions
test_arithmetic:
    ld d0, 10 + 20              ; 30
    ld d0, 100 - 50             ; 50
    ld d0, 8 * 8                ; 64
    ld d0, 256 / 4              ; 64
    ld d0, 100 % 7              ; 2

; Bitwise expressions
test_bitwise:
    ld d0, 0xFF00 & 0x0FF0      ; 0x0F00
    ld d0, 0x00F0 | 0x0F00      ; 0x0FF0
    ld d0, 0xAAAA ^ 0x5555      ; 0xFFFF
    ld d0, ~0                   ; 0xFFFFFFFF

; Shift expressions
test_shifts:
    ld d0, 1 << 0               ; 1
    ld d0, 1 << 8               ; 256
    ld d0, 1 << 16              ; 65536
    ld d0, 1 << 24              ; 16777216
    ld d0, 0x80000000 >> 16     ; 0x8000
    ld d0, 0xFF00 >> 4          ; 0x0FF0

; Exponentiation
test_exponent:
    ld d0, 2 ** 0               ; 1
    ld d0, 2 ** 1               ; 2
    ld d0, 2 ** 8               ; 256
    ld d0, 2 ** 16              ; 65536
    ld d0, 10 ** 3              ; 1000

; Unary expressions
test_unary:
    ld d0, -1                   ; 0xFFFFFFFF
    ld d0, -100                 ; negative
    ld d0, ~0xFF                ; 0xFFFFFF00
    ld d0, !0                   ; 1
    ld d0, !1                   ; 0

; Grouped expressions
test_grouped:
    ld d0, (10 + 20) * 3        ; 90
    ld d0, 10 + (20 * 3)        ; 70
    ld d0, ((5 + 3) * (2 + 2))  ; 32
    ld d0, (1 << 4) | (1 << 8)  ; 0x110

; Complex nested expressions
test_complex:
    ld d0, 1 + 2 * 3            ; 7 (mul before add)
    ld d0, (1 + 2) * 3          ; 9
    ld d0, 10 - 4 / 2           ; 8 (div before sub)
    ld d0, 0xFF & 0x0F | 0xF0   ; 0xFF

; Label arithmetic in expressions
expr_start:
    nop
    nop
    nop
    nop
expr_end:
    ld d0, expr_end - expr_start  ; Should be 8 (4 nops * 2 bytes)

; Data with expressions
.org 0x3000
computed_data:
.byte 2 ** 4, 2 ** 5, 2 ** 6, 2 ** 7
.word 1000 + 234, 0x1000 | 0x0234
.dword 0x12345678 & 0xFF00FF00
