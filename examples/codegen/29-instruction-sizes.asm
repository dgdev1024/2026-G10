; Test 29: Instruction Size Verification
; Tests that instructions are encoded with correct byte lengths.

.org 0x2000

; 2-byte instructions (opcode only)
size_2_bytes:
    nop                         ; 0x0000 - 2 bytes
    stop                        ; 0x0100 - 2 bytes
    mv d0, d1                   ; 0x3D01 - 2 bytes
    inc d0                      ; 0x6D00 - 2 bytes
    dec d0                      ; 0x6F00 - 2 bytes
    push d0                     ; 0x3C00 - 2 bytes
    pop d0                      ; 0x3600 - 2 bytes
    ret nc                      ; 0x4500 - 2 bytes
    sla l0                      ; 0x8000 - 2 bytes

; 3-byte instructions (opcode + imm8)
size_3_bytes:
    ld l0, 0x42                 ; 0x1000 + imm8 - 3 bytes
    ld h0, 0x42                 ; 0x1100 + imm8 - 3 bytes
    add l0, 0x10                ; 0x5000 + imm8 - 3 bytes
    sub l0, 0x10                ; 0x5600 + imm8 - 3 bytes
    and l0, 0xFF                ; 0x7000 + imm8 - 3 bytes
    or l0, 0x0F                 ; 0x7300 + imm8 - 3 bytes
    xor l0, 0xAA                ; 0x7600 + imm8 - 3 bytes
    cmp l0, 0x55                ; 0x7D00 + imm8 - 3 bytes
    ldp l0, [0x10]              ; 0x1500 + port - 3 bytes
    stp [0x20], l0              ; 0x1B00 + port - 3 bytes

; 4-byte instructions (opcode + imm16)
size_4_bytes:
    ld w0, 0x1234               ; 0x2000 + imm16 - 4 bytes
    add w0, 0x0100              ; 0x6000 + imm16 - 4 bytes
    sub w0, 0x0100              ; 0x6400 + imm16 - 4 bytes
    ldq d0, [0x1000]            ; 0x3300 + addr16 - 4 bytes
    stq [0x1000], d0            ; 0x3900 + addr16 - 4 bytes

; 4-byte instructions (opcode + simm16 for branches)
size_4_branch:
branch_target:
    jpb nc, branch_target       ; 0x4200 + offset16 - 4 bytes
    jpb zs, branch_target       ; 0x4210 + offset16 - 4 bytes

; 6-byte instructions (opcode + imm32)
size_6_bytes:
    ld d0, 0x12345678           ; 0x3000 + imm32 - 6 bytes
    add d0, 0x00001000          ; 0x6200 + imm32 - 6 bytes
    sub d0, 0x00001000          ; 0x6600 + imm32 - 6 bytes
    lsp 0x80000000              ; 0x3500 + imm32 - 6 bytes

; 6-byte instructions (opcode + addr32)
size_6_addr:
    ld d0, [0x80000000]         ; 0x3100 + addr32 - 6 bytes
    st [0x80000000], d0         ; 0x3700 + addr32 - 6 bytes
    ssp [0x80000000]            ; 0x3B00 + addr32 - 6 bytes
    jmp nc, size_6_addr         ; 0x4000 + addr32 - 6 bytes
    call nc, size_6_addr        ; 0x4300 + addr32 - 6 bytes

; Verify total size calculation
; Count the instructions and calculate expected section size
end_marker:
    nop
