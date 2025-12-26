; Test 21: Data Directives with Complex Expressions
; Tests data directives using complex expression values.
; Note: Use ROM region (below 0x80000000) for data values.

.org 0x2000

; Byte data with various expressions
byte_expressions:
.byte 1 + 1                             ; Simple addition
.byte 10 - 5                            ; Subtraction
.byte 2 * 3                             ; Multiplication
.byte 16 / 4                            ; Division
.byte 17 % 5                            ; Modulo
.byte 2 ** 4                            ; Exponentiation (16)
.byte 1 << 4                            ; Left shift (16)
.byte 0x80 >> 4                         ; Right shift (8)
.byte 0xFF & 0x0F                       ; Bitwise AND (15)
.byte 0xF0 | 0x0F                       ; Bitwise OR (255)
.byte 0xAA ^ 0x55                       ; Bitwise XOR (255)
.byte -1                                ; Unary negation
.byte ~0xF0                             ; Bitwise NOT
.byte !0                                ; Logical NOT (1)

; Word data with complex expressions
word_expressions:
.word 0x100 + 0x234                     ; Addition
.word 0x1000 - 0x100                    ; Subtraction
.word 0x10 * 0x10                       ; Multiplication (256)
.word 1 << 12                           ; Left shift (4096)
.word 0xFFFF & 0x0F0F                   ; Bitwise AND
.word 0xF0F0 | 0x0F0F                   ; Bitwise OR (0xFFFF)
.word 0xAAAA ^ 0x5555                   ; Bitwise XOR (0xFFFF)
.word (0x100 + 0x200) * 2               ; Grouped expression

; Dword data with complex expressions
dword_expressions:
.dword 0x10000 + 0x1234                 ; Addition
.dword 0x100000 - 0x10000               ; Subtraction
.dword 0x1000 * 0x1000                  ; Multiplication
.dword 1 << 24                          ; Left shift
.dword 0xFFFF0000 | 0x0000FFFF          ; Bitwise OR (0xFFFFFFFF)
.dword 0xDEAD << 16 | 0xBEEF            ; Build 0xDEADBEEF
.dword ~0                               ; All bits set
.dword 2 ** 20                          ; 1 MB (1048576)

; Multiple values with expressions
multi_byte_expr:
.byte 1+0, 1+1, 1+2, 1+3, 1+4

multi_word_expr:
.word 1<<8, 1<<9, 1<<10, 1<<11

multi_dword_expr:
.dword 1<<16, 1<<20, 1<<24, 1<<28

; Expression using other labels (forward reference)
.dword data_end - byte_expressions      ; Size calculation

data_end:
