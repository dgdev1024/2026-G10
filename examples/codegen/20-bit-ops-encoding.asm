; Test 20: Bit Test and Manipulation Encoding
; Tests encoding of bit test/set/reset/toggle instructions.
; Format: 0xA0XY BIT Y, LX (bit Y in register LX)

.org 0x2000

; BIT - Test Bit in Register
; Format: 0xA0XY BIT Y, LX (2 bytes)
test_bit_l:
    bit 0, l0                   ; 0xA000 - bit 0 of L0
    bit 1, l0                   ; 0xA001 - bit 1 of L0
    bit 7, l0                   ; 0xA007 - bit 7 of L0
    bit 0, l1                   ; 0xA010 - bit 0 of L1
    bit 3, l5                   ; 0xA053 - bit 3 of L5
    bit 7, l15                  ; 0xA0F7 - bit 7 of L15

; BIT - Test Bit at Memory Address
; Format: 0xA1XY BIT Y, [DX] (2 bytes)
test_bit_indirect:
    bit 0, [d0]                 ; 0xA100
    bit 7, [d0]                 ; 0xA107
    bit 3, [d5]                 ; 0xA153
    bit 7, [d15]                ; 0xA1F7

; SET - Set Bit in Register
; Format: 0xA2XY SET Y, LX (2 bytes)
test_set_l:
    set 0, l0                   ; 0xA200
    set 1, l0                   ; 0xA201
    set 7, l0                   ; 0xA207
    set 0, l1                   ; 0xA210
    set 5, l10                  ; 0xA2A5
    set 7, l15                  ; 0xA2F7

; SET - Set Bit at Memory Address
; Format: 0xA3XY SET Y, [DX] (2 bytes)
test_set_indirect:
    set 0, [d0]                 ; 0xA300
    set 7, [d0]                 ; 0xA307
    set 3, [d7]                 ; 0xA373
    set 7, [d15]                ; 0xA3F7

; RES - Reset (Clear) Bit in Register
; Format: 0xA4XY RES Y, LX (2 bytes)
test_res_l:
    res 0, l0                   ; 0xA400
    res 1, l0                   ; 0xA401
    res 7, l0                   ; 0xA407
    res 0, l1                   ; 0xA410
    res 4, l8                   ; 0xA484
    res 7, l15                  ; 0xA4F7

; RES - Reset Bit at Memory Address
; Format: 0xA5XY RES Y, [DX] (2 bytes)
test_res_indirect:
    res 0, [d0]                 ; 0xA500
    res 7, [d0]                 ; 0xA507
    res 5, [d10]                ; 0xA5A5
    res 7, [d15]                ; 0xA5F7

; TOG - Toggle Bit in Register
; Format: 0xA6XY TOG Y, LX (2 bytes)
test_tog_l:
    tog 0, l0                   ; 0xA600
    tog 1, l0                   ; 0xA601
    tog 7, l0                   ; 0xA607
    tog 0, l1                   ; 0xA610
    tog 6, l9                   ; 0xA696
    tog 7, l15                  ; 0xA6F7

; TOG - Toggle Bit at Memory Address
; Format: 0xA7XY TOG Y, [DX] (2 bytes)
test_tog_indirect:
    tog 0, [d0]                 ; 0xA700
    tog 7, [d0]                 ; 0xA707
    tog 3, [d7]                 ; 0xA773
    tog 7, [d15]                ; 0xA7F7

; Note: According to the G10 spec, bit operations only support
; L registers and [DX] indirect addressing, not W or D registers.
; The format is 0xAnXY where X=register index, Y=bit number (0-7).