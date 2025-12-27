; Test 12: Data Tables
; Tests: Larger data structures, arrays, lookup tables
;
; This program uses lookup tables for computation.

; Data section in ROM
.org 0x00003000

; Lookup table: squares of 0-9
square_table:
.dword 0                ; 0^2 = 0
.dword 1                ; 1^2 = 1
.dword 4                ; 2^2 = 4
.dword 9                ; 3^2 = 9
.dword 16               ; 4^2 = 16
.dword 25               ; 5^2 = 25
.dword 36               ; 6^2 = 36
.dword 49               ; 7^2 = 49
.dword 64               ; 8^2 = 64
.dword 81               ; 9^2 = 81

; Byte array
byte_array:
.byte 0x11, 0x22, 0x33, 0x44, 0x55

; Word array
word_array:
.word 0x1111, 0x2222, 0x3333

; BSS section in RAM
.org 0x80000000

result:
.dword 1

; Code section
.org 0x00002000

.global main

main:
    ; Look up square of 5
    ; Address = square_table + (5 * 4)
    ld d0, square_table     ; Base address
    ld d1, 20               ; 5 * 4 = 20 (offset for index 5)
    add d0, d1              ; D0 = address of square_table[5]
    ld d2, [d0]             ; D2 = 25 (square of 5)
    
    ; Store result
    st [result], d2
    
    ; Load from byte array
    ld l0, [byte_array]     ; L0 = 0x11
    
    ; Load from word array
    ld w0, [word_array]     ; W0 = 0x1111
    
    halt
