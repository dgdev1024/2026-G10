; Test 08: Branch and Jump Operations
; Tests JPB, JMP with various condition codes.
;
; Expected RAM layout at $80000000:
;   $00: 0x01          - JPB NC (unconditional) worked
;   $01: 0x02          - JPB ZS (jump on zero set) worked
;   $02: 0x03          - JPB ZC (jump on zero clear) worked
;   $03: 0x04          - JPB CS (jump on carry set) worked
;   $04: 0x05          - JPB CC (jump on carry clear) worked
;   $05: 0x06          - CMP and conditional branch worked
;   $06: 0x05          - Loop counter final value (counted 0 to 4 = 5)
;   $07: 0x08          - JMP absolute worked

.global main

; RAM section for test results
.org 0x80000000
    result_jpb_nc:      .byte 1
    result_jpb_zs:      .byte 1
    result_jpb_zc:      .byte 1
    result_jpb_cs:      .byte 1
    result_jpb_cc:      .byte 1
    result_cmp:         .byte 1
    result_loop:        .byte 1
    result_jmp:         .byte 1

; Code section
.org 0x2000
main:
    ; Test JPB NC (unconditional jump)
    jpb nc, test_nc_pass
    ld l0, 0x00         ; This should be skipped
    jpb nc, test_zs
test_nc_pass:
    ld l0, 0x01         ; JPB NC worked
    st [result_jpb_nc], l0
    
test_zs:
    ; Test JPB ZS (jump if zero flag set)
    ld l0, 0x00         ; Set zero flag by loading 0
    and l0, l0          ; AND with self to set zero flag
    jpb zs, test_zs_pass
    ld l0, 0x00         ; This should be skipped
    jpb nc, test_zc
test_zs_pass:
    ld l0, 0x02         ; JPB ZS worked
    st [result_jpb_zs], l0
    
test_zc:
    ; Test JPB ZC (jump if zero flag clear)
    ld l0, 0x42         ; Non-zero value
    and l0, l0          ; Clear zero flag
    jpb zc, test_zc_pass
    ld l0, 0x00         ; This should be skipped
    jpb nc, test_cs
test_zc_pass:
    ld l0, 0x03         ; JPB ZC worked
    st [result_jpb_zc], l0
    
test_cs:
    ; Test JPB CS (jump if carry flag set)
    ld l0, 0xFF
    ld l1, 0x01
    add l0, l1          ; 0xFF + 0x01 = overflow, sets carry
    jpb cs, test_cs_pass
    ld l0, 0x00         ; This should be skipped
    jpb nc, test_cc
test_cs_pass:
    ld l0, 0x04         ; JPB CS worked
    st [result_jpb_cs], l0
    
test_cc:
    ; Test JPB CC (jump if carry flag clear)
    ld l0, 0x01
    ld l1, 0x01
    add l0, l1          ; 0x01 + 0x01 = 0x02, no carry
    jpb cc, test_cc_pass
    ld l0, 0x00         ; This should be skipped
    jpb nc, test_cmp
test_cc_pass:
    ld l0, 0x05         ; JPB CC worked
    st [result_jpb_cc], l0
    
test_cmp:
    ; Test CMP and conditional branch
    ld l0, 0x10
    ld l1, 0x10
    cmp l0, l1          ; Compare equal, sets zero flag
    jpb zs, test_cmp_pass
    ld l0, 0x00         ; This should be skipped
    jpb nc, test_loop
test_cmp_pass:
    ld l0, 0x06         ; CMP + JPB ZS worked
    st [result_cmp], l0
    
test_loop:
    ; Test loop with counter
    ld l0, 0x00         ; Counter
    ld l1, 0x05         ; Target
loop_start:
    inc l0              ; Increment counter
    cmp l0, l1          ; Compare with target
    jpb zc, loop_start  ; Loop while not equal
    st [result_loop], l0  ; Store final counter value (should be 5)
    
test_jmp:
    ; Test JMP (absolute jump)
    jmp test_jmp_pass
    ld l0, 0x00         ; This should be skipped
    jpb nc, test_end
test_jmp_pass:
    ld l0, 0x08         ; JMP worked
    st [result_jmp], l0
    
test_end:
    ; End program
    stop
