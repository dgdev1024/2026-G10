; Test 15: Call and Return Encoding
; Tests encoding of subroutine call and return instructions.

.org 0x2000

; CALL - Call Subroutine (immediate address)
; Format: 0x43X0 CALL X, IMM32 (6 bytes)
subroutine:
    nop
    ret nc

test_call_imm:
    call nc, subroutine         ; Condition 0
    call zs, subroutine         ; Condition 1
    call zc, subroutine         ; Condition 2
    call cs, subroutine         ; Condition 3
    call cc, subroutine         ; Condition 4
    call vs, subroutine         ; Condition 5
    call vc, subroutine         ; Condition 6

; INT - Software Interrupt (call interrupt vector)
; Format: 0x44XX INT XX (2 bytes) - calls interrupt vector XX (0-31)
test_int:
    int 0x00                    ; 0x4400 - call interrupt 0
    int 0x01                    ; 0x4401 - call interrupt 1
    int 0x0F                    ; 0x440F - call interrupt 15
    int 0x10                    ; 0x4410 - call interrupt 16
    int 0x1F                    ; 0x441F - call interrupt 31 (max)

; RET - Return from Subroutine
; Format: 0x45X0 RET X (2 bytes)
test_ret:
    ret nc                      ; 0x4500 - unconditional return
    ret zs                      ; 0x4510
    ret zc                      ; 0x4520
    ret cs                      ; 0x4530
    ret cc                      ; 0x4540
    ret vs                      ; 0x4550
    ret vc                      ; 0x4560

; RETI - Return from Interrupt
; Format: 0x46X0 RETI X (2 bytes)
test_reti:
    reti nc                     ; 0x4600 - unconditional
    reti zs                     ; 0x4610
    reti zc                     ; 0x4620
    reti cs                     ; 0x4630
    reti cc                     ; 0x4640
    reti vs                     ; 0x4650
    reti vc                     ; 0x4660
