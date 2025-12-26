; Test 24: Label Resolution and Symbol Encoding
; Tests that labels are correctly resolved to addresses.

.org 0x2000

; Forward reference labels
start:
    jmp nc, forward_label       ; Reference before definition
    jpb nc, another_forward     ; Branch to forward label
    call nc, forward_subroutine

; Labels at known offsets
label_at_2000:                  ; Address 0x2000 + some offset
    nop

label_with_code:
    ld d0, 0x12345678           ; 6 bytes
next_label:
    ld d1, 0xDEADBEEF           ; 6 bytes
another_label:
    nop                         ; 2 bytes

; Forward labels (now defined)
forward_label:
    nop
another_forward:
    nop
forward_subroutine:
    ret nc

; Backward references
backward_refs:
    jmp nc, start               ; Jump backward
    jpb nc, label_at_2000       ; Branch backward
    call nc, forward_subroutine ; Call (already defined)

; Label used as immediate value
load_label_address:
    ld d0, start                ; Load address of 'start' label
    ld d1, forward_label        ; Load address of forward label
    ld d2, backward_refs        ; Load address of backward refs

; Label arithmetic
label_math:
    ld d0, forward_label - start           ; Difference between labels
    ld d0, next_label - label_with_code    ; Should be 6 (size of ld d0, imm32)

; Multiple labels at same address
multi_label_1:
multi_label_2:
multi_label_3:
    nop

; Labels referenced in data
.org 0x2200
label_table:
.dword start
.dword forward_label
.dword backward_refs
.dword multi_label_1
