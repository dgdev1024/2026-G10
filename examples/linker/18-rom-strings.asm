; Test 18: ROM Data Strings
; Tests: String data storage, string literals
;
; This program stores and accesses string data.

; Data section in ROM
.org 0x00003000

; String table
hello_str:
.byte 'H', 'e', 'l', 'l', 'o', 0

world_str:
.byte 'W', 'o', 'r', 'l', 'd', '!', 0

; Numeric string
number_str:
.byte '1', '2', '3', '4', '5', 0

; String lengths
hello_len:
.byte 5
world_len:
.byte 6
number_len:
.byte 5

; BSS section in RAM
.org 0x80000000

string_ptr:
.dword 1

; Code section
.org 0x00002000

.global main

main:
    ; Load address of hello string
    ld d0, hello_str
    st [string_ptr], d0
    
    ; Load first character
    ld l0, [hello_str]      ; L0 = 'H' = 0x48
    
    ; Load string length
    ld l1, [hello_len]      ; L1 = 5
    
    ; Calculate end of string address using D0 as accumulator
    ld d0, hello_str        ; D0 = base address
    ld d1, 0
    mv l1, l1               ; Keep L1 as is (length)
    ; For proper addition, use D0 as accumulator
    ; D0 = hello_str + 5 (length to find null terminator)
    add d0, d1              ; D0 = address + D1 (but D1 is 0, so D0 unchanged)
    ; Actually, let's just load the 5th offset address directly
    ld d0, hello_str
    ld d1, 5
    add d0, d1              ; D0 = hello_str + 5 = null terminator address
    
    ; Verify null terminator
    ld l3, [d0]             ; L3 should be 0
    
    halt
