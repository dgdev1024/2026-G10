; Test 19: Shift and Rotate Instruction Encoding
; Tests encoding of bit shift and rotation instructions.
; Note: Shifts and rotates only work on L registers and [DX] indirect.
; SWAP works on L, W, and D registers.

.org 0x2000

; SLA - Shift Left Arithmetic (8-bit register)
; Format: 0x80X0 SLA LX (2 bytes)
test_sla_l:
    sla l0                      ; 0x8000
    sla l1                      ; 0x8010
    sla l7                      ; 0x8070
    sla l15                     ; 0x80F0

; SLA - Shift Left Arithmetic (indirect)
; Format: 0x81X0 SLA [DX] (2 bytes)
test_sla_indirect:
    sla [d0]                    ; 0x8100
    sla [d7]                    ; 0x8170
    sla [d15]                   ; 0x81F0

; SRA - Shift Right Arithmetic (8-bit register)
; Format: 0x82X0 SRA LX (2 bytes)
test_sra_l:
    sra l0                      ; 0x8200
    sra l1                      ; 0x8210
    sra l15                     ; 0x82F0

; SRA - Shift Right Arithmetic (indirect)
; Format: 0x83X0 SRA [DX] (2 bytes)
test_sra_indirect:
    sra [d0]                    ; 0x8300
    sra [d7]                    ; 0x8370
    sra [d15]                   ; 0x83F0

; SRL - Shift Right Logical (8-bit register)
; Format: 0x84X0 SRL LX (2 bytes)
test_srl_l:
    srl l0                      ; 0x8400
    srl l7                      ; 0x8470
    srl l15                     ; 0x84F0

; SRL - Shift Right Logical (indirect)
; Format: 0x85X0 SRL [DX] (2 bytes)
test_srl_indirect:
    srl [d0]                    ; 0x8500
    srl [d7]                    ; 0x8570
    srl [d15]                   ; 0x85F0

; SWAP - Swap nibbles in L register
; Format: 0x86X0 SWAP LX (2 bytes)
test_swap_l:
    swap l0                     ; 0x8600
    swap l7                     ; 0x8670
    swap l15                    ; 0x86F0

; SWAP - Swap nibbles at memory
; Format: 0x87X0 SWAP [DX] (2 bytes)
test_swap_indirect:
    swap [d0]                   ; 0x8700
    swap [d7]                   ; 0x8770
    swap [d15]                  ; 0x87F0

; SWAP - Swap bytes in W register
; Format: 0x88X0 SWAP WX (2 bytes)
test_swap_w:
    swap w0                     ; 0x8800
    swap w7                     ; 0x8870
    swap w15                    ; 0x88F0

; SWAP - Swap words in D register
; Format: 0x89X0 SWAP DX (2 bytes)
test_swap_d:
    swap d0                     ; 0x8900
    swap d7                     ; 0x8970
    swap d15                    ; 0x89F0

; RLA - Rotate Left Accumulator through Carry
; Format: 0x9000 RLA (2 bytes)
test_rla:
    rla                         ; 0x9000

; RL - Rotate Left through Carry (8-bit register)
; Format: 0x91X0 RL LX (2 bytes)
test_rl_l:
    rl l0                       ; 0x9100
    rl l7                       ; 0x9170
    rl l15                      ; 0x91F0

; RLCA - Rotate Left Accumulator Circular
; Format: 0x9300 RLCA (2 bytes)
test_rlca:
    rlca                        ; 0x9300

; RLC - Rotate Left Circular (8-bit register)
; Format: 0x94X0 RLC LX (2 bytes)
test_rlc_l:
    rlc l0                      ; 0x9400
    rlc l7                      ; 0x9470
    rlc l15                     ; 0x94F0

; RRA - Rotate Right Accumulator through Carry
; Format: 0x9600 RRA (2 bytes)
test_rra:
    rra                         ; 0x9600

; RR - Rotate Right through Carry (8-bit register)
; Format: 0x97X0 RR LX (2 bytes)
test_rr_l:
    rr l0                       ; 0x9700
    rr l7                       ; 0x9770
    rr l15                      ; 0x97F0

; RRCA - Rotate Right Accumulator Circular
; Format: 0x9900 RRCA (2 bytes)
test_rrca:
    rrca                        ; 0x9900

; RRC - Rotate Right Circular (8-bit register)
; Format: 0x9AX0 RRC LX (2 bytes)
test_rrc_l:
    rrc l0                      ; 0x9A00
    rrc l7                      ; 0x9A70
    rrc l15                     ; 0x9AF0
