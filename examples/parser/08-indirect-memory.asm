; Test 08: Indirect Memory Operands
; Tests the parser's ability to parse indirect memory addressing operands.
; Indirect addressing uses a register as the memory address pointer [reg].

.org 0x2000

; Indirect addressing with 32-bit dword registers
test_dword_indirect:
    ld d0, [d1]
    ld d0, [d2]
    ld d0, [d15]
    st [d1], d0
    st [d2], d0
    st [d15], d0

; Indirect addressing with 16-bit word registers
test_word_indirect:
    ld w0, [w1]
    ld w0, [w2]
    ld w0, [w15]
    st [w1], w0
    st [w2], w0
    st [w15], w0

; Indirect addressing with 8-bit low registers
test_byte_indirect:
    ld l0, [l1]
    ld l0, [l2]
    ld l0, [l15]
    st [l1], l0
    st [l2], l0
    st [l15], l0

; Mixed: load different sizes from indirect address
test_mixed_indirect:
    ld d0, [d1]     ; Load dword from address in d1
    ld w0, [d1]     ; Load word from address in d1
    ld l0, [d1]     ; Load byte from address in d1
