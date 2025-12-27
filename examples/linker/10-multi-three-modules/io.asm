; Test 10: Three Modules - IO Module
; Tests: I/O functions for storing output

; BSS section in RAM
.org 0x80000100

.global output_buffer
output_buffer:
.dword 1

; Code section
.org 0x00002300

; External reference to result from main
.extern result_value

.global store_output

; Function: store_output
; Copies result_value to output_buffer
store_output:
    ld d0, [result_value]
    st [output_buffer], d0
    ret
