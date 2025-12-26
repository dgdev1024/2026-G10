; Test 24: Complete Program Structure
; Tests a complete, realistic assembly program structure with all features.

; Module exports and imports
.global main
.global helper_function
.extern external_library_func
.extern io_write

; Constants section (using expressions)
BUFFER_SIZE:
.dword 256
SCREEN_WIDTH:
.dword 80
SCREEN_HEIGHT:
.dword 25
SCREEN_SIZE:
.dword 80 * 25
IO_PORT_BASE:
.dword 0xFFFFFF00
STATUS_REG:
.dword 0x10
DATA_REG:
.dword 0x20

; Interrupt vectors
.org 0x00001000
interrupt_table:
.dword irq_handler_0
.dword irq_handler_1
.dword irq_handler_2
.dword irq_handler_3

; Program code
.org 0x00002000

main:
    ; Initialize stack pointer using spi (set SP from register)
    ld d0, stack_top
    spi d0
    
    ; Initialize registers
    ld d0, 0
    ld d1, 0
    ld d2, 0
    
    ; Call initialization
    call nc, init_hardware
    
    ; Main loop
main_loop:
    ; Check for input (using IO port address directly)
    ldp l0, [0x10]
    bit 0, l0                   ; Check ready bit
    jpb zc, main_loop           ; Loop if not ready
    
    ; Read data
    ldp l1, [0x20]
    
    ; Process data
    call nc, process_byte
    
    ; Store result
    st [result_buffer], d0
    
    ; Continue loop
    jpb nc, main_loop

init_hardware:
    ; Disable interrupts during init
    di
    
    ; Configure I/O ports (using direct addresses)
    ld l0, 0xFF
    stp [0x10], l0
    
    ; Clear data register
    ld l0, 0
    stp [0x20], l0
    
    ; Enable interrupts
    ei
    
    ret nc

process_byte:
    ; Input in l1, output in d0
    ld d0, 0
    mv l0, l1
    
    ; Apply transformation
    and l0, 0x7F                ; Mask to 7 bits
    ; Note: Cannot directly move L to D, use as is in L0
    
    ret nc

helper_function:
    ; Save registers
    push d1
    push d2
    
    ; Perform calculations using D0 operations
    mv d1, d0
    ld d2, 10
    
    ; Multiply by 10
    add d0, d0                  ; d0 * 2
    add d0, d0                  ; d0 * 4
    add d0, d1                  ; d0 * 5
    add d0, d0                  ; d0 * 10
    
    ; Restore registers
    pop d2
    pop d1
    
    ret nc

; Interrupt handlers
irq_handler_0:
    push d0
    ld d0, 0
    ; Handle IRQ 0
    pop d0
    reti nc

irq_handler_1:
    push d0
    ld d0, 1
    ; Handle IRQ 1
    pop d0
    reti nc

irq_handler_2:
    push d0
    ld d0, 2
    ; Handle IRQ 2
    pop d0
    reti nc

irq_handler_3:
    push d0
    ld d0, 3
    ; Handle IRQ 3
    pop d0
    reti nc

; Data section (RAM region - .byte/.word/.dword are reservations, not values)
.org 0x80000000

result_buffer:
.dword 1                        ; Reserve 1 dword

data_buffer:
.byte 16                        ; Reserve 16 bytes

; String data goes in ROM section instead
.org 0x3000
message:
.byte 'H', 'e', 'l', 'l', 'o', 0

; Lookup table (in ROM)
lookup_table:
.byte 0, 1, 2, 3, 4, 5, 6, 7
.byte 8, 9, 10, 11, 12, 13, 14, 15

; Stack area (RAM)
.org 0x80010000
stack_bottom:
.dword 1                        ; Reserve 1 dword as marker
; Reserve 4KB for stack
.org 0x80011000
stack_top:
