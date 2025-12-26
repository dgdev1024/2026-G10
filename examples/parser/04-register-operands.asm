; Test 04: Register Operands
; Tests the parser's ability to parse register operands.
; The G10 CPU has 32-bit (D), 16-bit (W), and 8-bit (H/L) registers.

.org 0x2000

; 32-bit dword registers (D0-D15)
test_dword_regs:
    inc d0
    inc d1
    inc d2
    inc d3
    inc d4
    inc d5
    inc d6
    inc d7
    inc d8
    inc d9
    inc d10
    inc d11
    inc d12
    inc d13
    inc d14
    inc d15

; 16-bit word registers (W0-W15)
test_word_regs:
    inc w0
    inc w1
    inc w2
    inc w7
    inc w15

; 8-bit high byte registers (H0-H15)
test_high_regs:
    inc h0
    inc h1
    inc h7
    inc h15

; 8-bit low byte registers (L0-L15)
test_low_regs:
    inc l0
    inc l1
    inc l7
    inc l15

; Mixed register sizes in different instructions
test_mixed:
    mv d0, d1
    mv w0, w1
    mv l0, l1
    mv h0, l1       ; Move L to H (no direct H to H move exists)
