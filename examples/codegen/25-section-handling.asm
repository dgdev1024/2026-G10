; Test 25: Section Handling
; Tests that multiple sections are correctly generated.

; Section 1: Program metadata (low ROM)
.org 0x0000
metadata_section:
.dword 0x10325476               ; Magic number
.dword 0x00010000               ; Version
.dword entry_point              ; Entry point address
.dword 0x00000000               ; Reserved

; Section 2: Interrupt vector table
.org 0x1000
interrupt_table:
.dword irq_handler_0
.dword irq_handler_1
.dword irq_handler_2
.dword irq_handler_3
.dword irq_handler_4
.dword irq_handler_5
.dword irq_handler_6
.dword irq_handler_7

; Section 3: Main code section
.org 0x2000
entry_point:
    lsp 0x80010000              ; Initialize stack
    call nc, main               ; Call main function
    stop                        ; Halt on return

main:
    ld d0, 0
    ld d1, 10
main_loop:
    inc d0
    cmp l0, l1
    jpb zc, main_loop
    ret nc

; Interrupt handlers
irq_handler_0:
    push d0
    ld d0, 0
    pop d0
    reti nc

irq_handler_1:
    push d0
    ld d0, 1
    pop d0
    reti nc

irq_handler_2:
    reti nc

irq_handler_3:
    reti nc

irq_handler_4:
    reti nc

irq_handler_5:
    reti nc

irq_handler_6:
    reti nc

irq_handler_7:
    reti nc

; Section 4: Read-only data
.org 0x3000
rodata_section:
.byte 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0
.byte 0, 0, 0                   ; Padding

lookup_table:
.byte 0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 169, 196, 225

; Section 5: RAM section (BSS - reservations only)
.org 0x80000000
bss_section:
.dword 1                        ; Reserve 1 dword for result
.byte 256                       ; Reserve 256 bytes for buffer

; Section 6: Stack section
.org 0x80010000
stack_bottom:
.dword 1                        ; Stack bottom marker
