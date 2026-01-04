; Test 4: Mixed Content
; This test verifies a combination of regular code, comments, and
; line continuations.

.org 0x0000

; Initialize registers
start:
    ld r0, 0x00         ; Load zero into r0
    ld r1, 0x01         ; Load one into r1

; A long instruction with line continuation
    add r0, \
        r1              ; Add r1 to r0

; String literal test
    .byte "Hello, World!"
    .byte 'A'

; More operations
loop:
    sub r0, r1
    jnz loop

    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Mixed Content
;
; This test verifies that the preprocessor correctly handles a combination
; of different token types including:
; - Labels (with colons)
; - Instructions with register operands
; - Numeric literals (hex)
; - String and character literals
; - Comments (should be stripped by lexer)
; - Line continuations
;
; EXPECTED OUTPUT:
;
; .org 0x0000
; start:
; ld r0, 0x00
; ld r1, 0x01
; add r0, r1
; .byte "Hello, World!"
; .byte 'A'
; loop:
; sub r0, r1
; jnz loop
; halt
;
;----------------------------------------------------------------------------
