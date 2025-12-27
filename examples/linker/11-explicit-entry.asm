; Test 11: Explicit Entry Point with _start
; Tests: Using _start as alternative entry point
;
; This program uses _start instead of main as the entry point.

.org 0x00002000

; Declare _start as global
.global _start

_start:
    ; Initialize registers
    ld d0, 0
    ld d1, 0
    ld d2, 0
    
    ; Call our real main logic
    call nc, program_main
    
    ; Halt after program completes
    halt

program_main:
    ; Some program logic
    ld d0, 100
    ld d1, 200
    add d0, d1              ; D0 = 300
    ret

; This function is not called but should still be linked
unused_function:
    ld d0, 0xDEAD
    ret
