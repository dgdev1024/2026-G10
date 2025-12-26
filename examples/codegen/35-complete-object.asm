; Test 35: Complete Object File Generation
; A comprehensive test that exercises all major codegen features.
; Designed to produce a valid, complete G10 object file.

; Module exports
.global _start
.global main
.global calculate

; External references
.extern printf
.extern malloc

; Program metadata section
.org 0x0000
program_header:
.dword 0x10325476               ; Magic: "G10P"
.dword 0x00010000               ; Version 1.0.0
.dword _start                   ; Entry point
.dword main                     ; Main function

; Interrupt Vector Subroutines (IVT)
; Each interrupt has 0x80 bytes reserved in 0x1000-0x1FFF.
; Place the handlers directly at the IVT addresses.
.org 0x1000                 ; Interrupt #0 (exception handler)
isr0:
    ; Minimal handler: just return from interrupt
    reti nc

.org 0x1080                 ; Interrupt #1
isr1:
    reti nc

.org 0x1100                 ; Interrupt #2
isr2:
    reti nc

.org 0x1180                 ; Interrupt #3
isr3:
    reti nc

; Main code section
.org 0x2000

_start:
    ; Initialize stack
    lsp 0x80010000
    
    ; Clear registers
    ld d0, 0
    ld d1, 0
    ld d2, 0
    ld d3, 0
    
    ; Call main
    call nc, main
    
    ; Exit
    stop

main:
    ; Save registers
    push d1
    push d2
    
    ; Load parameters
    ld d1, 10
    ld d2, 20
    
    ; Calculate sum
    call nc, calculate
    
    ; Result in d0
    
    ; Restore and return
    pop d2
    pop d1
    ret nc

calculate:
    ; Add d1 + d2, result in d0
    mv d0, d1
    add d0, d2
    ret nc

; Default interrupt handler (used by code flow, outside IVT region)
default_handler:
    reti nc

; Utility functions
utility_section:
    
; Increment counter at address in d1
increment_counter:
    ld d0, [d1]
    inc d0
    st [d1], d0
    ret nc

; Clear buffer: d1 = address, d2 = count
clear_buffer:
    ld l0, 0
clear_loop:
    st [d1], l0
    inc d1
    dec d2
    jpb zc, clear_loop
    ret nc

; Read-only data section
.org 0x3000
rodata:

message_hello:
.byte 'H', 'e', 'l', 'l', 'o', ',', ' '
.byte "World!", 0x00

message_error:
.byte 'E', 'r', 'r', 'o', 'r', 0

lookup_squares:
.byte 0, 1, 4, 9, 16, 25, 36, 49
.byte 64, 81, 100, 121, 144, 169, 196, 225

constants:
.dword 0x00000000               ; Zero
.dword 0x00000001               ; One
.dword 0xFFFFFFFF               ; All ones
.dword 0x80000000               ; Min signed
.dword 0x7FFFFFFF               ; Max signed

; Address table
function_table:
.dword main
.dword calculate
.dword increment_counter
.dword clear_buffer
.dword 0                        ; Placeholder (external printf removed)
.dword 0                        ; Placeholder (external malloc removed)

; RAM section (BSS)
.org 0x80000000
bss:

; Global variables
counter:
.dword 1                        ; Reserve 1 dword

result:
.dword 1                        ; Reserve 1 dword

buffer:
.byte 256                       ; Reserve 256 bytes

; Stack
.org 0x80010000
stack_base:
.dword 1                        ; Stack marker
