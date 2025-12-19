; Test 1: Basic Lexing - Keywords, Identifiers, and Simple Constructs
; This test verifies the lexer can handle basic assembly constructs including
; directives, labels, instructions, registers, and condition codes.

.ORG $2000              ; Directive with hexadecimal literal

main:                   ; Label definition
    LD L0, 42           ; Load immediate value into accumulator
    ST [$80000000], L0  ; Store to RAM address
    MV D1, D0           ; Move register to register

loop:
    INC W1              ; Increment 16-bit register
    CMP W0, 100         ; Compare with decimal literal
    JPB ZC, loop        ; Conditional relative jump (jump if zero clear)

done:
    JPB NC, done        ; Infinite loop (no condition - always jump)
    HALT                ; Stop execution
