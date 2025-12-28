; Test 11: Interrupt Handling
; Tests the interrupt registers IE, IRQ and interrupt enable/disable.
;
; Interrupt I/O Port Addresses (relative to $FFFFFF00):
;   IRQ0 = $00 ($FFFFFF00) - IRQ byte 0 (interrupts 0-7)
;   IRQ1 = $01 ($FFFFFF01) - IRQ byte 1 (interrupts 8-15)
;   IRQ2 = $02 ($FFFFFF02) - IRQ byte 2 (interrupts 16-23)
;   IRQ3 = $03 ($FFFFFF03) - IRQ byte 3 (interrupts 24-31)
;   IE0  = $04 ($FFFFFF04) - IE byte 0 (interrupts 0-7)
;   IE1  = $05 ($FFFFFF05) - IE byte 1 (interrupts 8-15)
;   IE2  = $06 ($FFFFFF06) - IE byte 2 (interrupts 16-23)
;   IE3  = $07 ($FFFFFF07) - IE byte 3 (interrupts 24-31)
;
; Expected RAM layout at $80000000:
;   $00: 0x0F         - IE0 write/read (enable interrupts 0-3)
;   $01: 0x00         - IE1 write/read (no interrupts enabled)
;   $02: 0xAA         - IRQ0 write/read test
;   $03: 0x00         - IRQ0 after clearing
;   $04: 0x01         - Interrupt handler was called (if supported)

.global main

; RAM section for test results
.ram
    result_ie0:         .byte 1
    result_ie1:         .byte 1
    result_irq_write:   .byte 1
    result_irq_clear:   .byte 1
    result_handler:     .byte 1

; Interrupt vector table starts at $1000
; Each vector is 0x80 (128) bytes apart
; Vector 0: $1000 (exception handler)
; Vector 1: $1080 (reserved)
; Vector 2: $1100 (reserved)
; Vector 3: $1180 (timer interrupt)
.int 3
timer_isr:
    ; Timer interrupt handler
    ; Set flag to indicate we were called
    push d0
    ld l0, 0x01
    st [result_handler], l0
    pop d0
    stop

; Code section
.rom
main:
    ; Initialize result_handler to 0
    ld l0, 0x00
    st [result_handler], l0
    
    ; Test IE0 write/read (enable interrupts 0-3)
    ; Note: Bit 0 (exception handler) is always enabled
    ld l0, 0x0F
    stp [0x04], l0      ; Write to IE0
    ldp l0, [0x04]      ; Read IE0 back
    st [result_ie0], l0
    
    ; Test IE1 write/read (no interrupts enabled)
    ld l0, 0x00
    stp [0x05], l0      ; Write to IE1
    ldp l0, [0x05]      ; Read IE1 back
    st [result_ie1], l0
    
    ; Test IRQ0 write/read
    ld l0, 0xAA
    stp [0x00], l0      ; Write 0xAA to IRQ0
    ldp l0, [0x00]      ; Read IRQ0 back
    st [result_irq_write], l0
    
    ; Clear IRQ0
    ld l0, 0x00
    stp [0x00], l0      ; Clear IRQ0
    ldp l0, [0x00]      ; Read IRQ0 back
    st [result_irq_clear], l0
    
    ; Note: Actually triggering an interrupt and handling it
    ; requires the EI instruction and proper timing.
    ; For this basic test, we just verify register access.
    
    ; Enable interrupts, then wait.
    ld l0, 0b1000
    stp [0x00], l0
    ei

wait:
    jpb wait
