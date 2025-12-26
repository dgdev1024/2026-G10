; Test 26: Global Symbol Export
; Tests that global symbols are properly exported in the symbol table.

.global main
.global helper_function
.global data_buffer
.global constant_value

.org 0x2000

; Global entry point
main:
    call nc, helper_function
    ld d0, constant_value       ; Load address of constant
    ld d1, [constant_value]     ; Load value at constant
    ret nc

; Global helper function
helper_function:
    ld d0, 42
    ret nc

; Local function (not exported)
local_function:
    inc d0
    ret nc

; Another local
internal_helper:
    dec d0
    ret nc

; Global data
.org 0x3000
constant_value:
.dword 0x12345678

data_buffer:
.byte 'G', '1', '0', 0
.byte 0, 0, 0, 0                ; Padding

; Local data (not exported)
local_data:
.dword 0xDEADBEEF
