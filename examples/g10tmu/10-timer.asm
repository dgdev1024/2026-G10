; Test 10: Timer Hardware
; Tests the timer registers DIV, TIMA, TMA, TAC.
;
; Timer I/O Port Addresses (relative to $FFFFFF00):
;   DIV  = $09 ($FFFFFF09)
;   TIMA = $0A ($FFFFFF0A)
;   TMA  = $0B ($FFFFFF0B)
;   TAC  = $0C ($FFFFFF0C)
;
; Expected RAM layout at $80000000:
;   $00: Non-zero     - DIV read (should be non-zero after some cycles)
;   $01: 0x00         - DIV after reset (write resets to 0)
;   $02: 0x42         - TIMA write/read test
;   $03: 0xAB         - TMA write/read test
;   $04: 0xFD         - TAC write/read (0xFD = bits 3-7 as 1, bit 2=1 enable, bits 0-1=01)
;   $05: 0x01         - Timer enabled check (1=yes, 0=no)

.global main

; RAM section for test results
.org 0x80000000
    result_div_read:    .byte 1
    result_div_reset:   .byte 1
    result_tima:        .byte 1
    result_tma:         .byte 1
    result_tac:         .byte 1
    result_enabled:     .byte 1

; Code section
.org 0x2000
main:
    ; First, let some time pass for DIV to increment
    ; (DIV increments every 256 T-cycles based on the internal divider)
    ld l0, 0x00         ; Loop counter
div_wait_loop:
    inc l0
    cmp l0, 0x80        ; Loop 128 times
    jpb zc, div_wait_loop
    
    ; Test DIV read - should be non-zero after waiting
    ldp l0, [0x09]      ; Read DIV
    st [result_div_read], l0
    
    ; Test DIV reset - writing any value resets it to 0
    ld l0, 0xFF
    stp [0x09], l0      ; Write to DIV (value ignored, resets to 0)
    ldp l0, [0x09]      ; Read DIV immediately (should be 0 or very small)
    st [result_div_reset], l0
    
    ; Test TIMA write/read
    ld l0, 0x42
    stp [0x0A], l0      ; Write 0x42 to TIMA
    ldp l0, [0x0A]      ; Read TIMA back
    st [result_tima], l0
    
    ; Test TMA write/read
    ld l0, 0xAB
    stp [0x0B], l0      ; Write 0xAB to TMA
    ldp l0, [0x0B]      ; Read TMA back
    st [result_tma], l0
    
    ; Test TAC write/read
    ; TAC format: bits 7-3 unused (read as 1), bit 2=enable, bits 1-0=clock select
    ; Write 0x05 = 0b00000101 -> should read as 0b11111101 = 0xFD
    ld l0, 0x05         ; Enable=1, Clock=01 (262144 Hz)
    stp [0x0C], l0      ; Write to TAC
    ldp l0, [0x0C]      ; Read TAC back
    st [result_tac], l0
    
    ; Check if timer is enabled (bit 2 of TAC)
    ldp l0, [0x0C]      ; Read TAC
    and l0, 0x04        ; Mask bit 2
    ld l1, 0x00         ; Default: not enabled
    cmp l0, 0x00
    jpb zs, timer_disabled
    ld l1, 0x01         ; Timer is enabled
timer_disabled:
    st [result_enabled], l1
    
    ; End program
    stop
