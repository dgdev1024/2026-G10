; Test 08: Store Instruction Encoding
; Tests encoding of store instructions (direct and indirect).

.org 0x2000

; 32-bit store to direct address
; Format: 0x370Y ST [ADDR32], DY (6 bytes)
test_st_direct:
    st [0x80000000], d0         ; 0x3700 + address
    st [0x80000004], d1         ; 0x3701 + address
    st [0x80000008], d7         ; 0x3707 + address
    st [0x8000000C], d15        ; 0x370F + address

; 32-bit store to indirect address
; Format: 0x38XY ST [DX], DY (2 bytes)
test_st_indirect:
    st [d0], d0                 ; 0x3800
    st [d0], d1                 ; 0x3801
    st [d1], d0                 ; 0x3810
    st [d15], d15               ; 0x38FF
    st [d7], d3                 ; 0x3873

; 8-bit store to direct address
test_st8_direct:
    st [0x80000000], l0         ; 0x1700 + address
    st [0x80000001], l5         ; 0x1705 + address
    st [0x80000002], h0         ; 0x1800 + address

; 8-bit store to indirect address
test_st8_indirect:
    st [d0], l0                 ; 0x190Y format
    st [d1], l5
    st [d0], h0                 ; 0x1A0Y format

; 16-bit store to direct address
test_st16_direct:
    st [0x80000000], w0         ; 0x2700 + address
    st [0x80000010], w7         ; 0x2707 + address

; 16-bit store to indirect address
test_st16_indirect:
    st [d0], w0                 ; 0x280Y format
    st [d7], w15
