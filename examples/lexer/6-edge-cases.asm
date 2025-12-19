; Test 6: Edge Cases and Error Conditions
; This test file contains various edge cases to stress-test the lexer.

.ORG 0x8000

edge_cases:
    ; Multiple spaces and tabs
    LD      L0  ,   42          ; Extra whitespace

    ; Empty lines and multiple newlines


    NOP                         ; After multiple blank lines

    ; Comments in various positions
    LD L1, 10       ; End of line comment
    ; Full line comment
        ; Indented comment
    LD L2, 20 ; Comment ; with ; semicolons

    ; Mixed case identifiers
    MyLabel:
    another_label_123:
    LABEL_WITH_CAPS:

    ; Long identifiers
    this_is_a_very_long_label_name_to_test_lexer_limits:

    ; Identifiers with periods (common in assembly)
    .local_label:
    local.subroutine:
    module.function.name:

    ; Nested brackets and parentheses
    ; LD D0, [[0x9000]]            ; Nested brackets (not valid in G10 assembly language)
    ; LD D1, ((5 + 3) * 2)      ; Nested parentheses in expressions

    ; Adjacent operators
    ; a+=b                      ; No spaces
    ; c >>= 3                   ; Shift assignment

    ; Large numbers
    LD D2, 2147483647           ; Max 32-bit signed int
    LD D3, 0xFFFFFFFF           ; Max 32-bit unsigned
    LD D4, 0b11111111111111111111111111111111  ; 32 bits in binary

    ; Zero in different bases
    LD L5, 0                    ; Decimal zero
    LD L6, 0x0                  ; Hex zero
    LD L7, 0b0                  ; Binary zero
    LD L8, 0o0                  ; Octal zero

    ; Single character labels
    a:
    X:
    _:

    ; Registers of different sizes
    LD D0, D15                  ; 32-bit registers
    LD W0, W15                  ; 16-bit registers
    LD H0, H15                  ; 8-bit high registers
    LD L0, L15                  ; 8-bit low registers

    ; All condition codes
    JPB NC, target_nc           ; No condition
    JPB ZS, target_zs           ; Zero set
    JPB ZC, target_zc           ; Zero clear
    JPB CS, target_cs           ; Carry set
    JPB CC, target_cc           ; Carry clear
    JPB VS, target_vs           ; Overflow set
    JPB VC, target_vc           ; Overflow clear

target_nc:
target_zs:
target_zc:
target_cs:
target_cc:
target_vs:
target_vc:
    RET

    ; Maximum length line (test buffer limits)
    LD D0, 0x12345678 ; This is a very long comment that extends far beyond what would normally be reasonable in assembly code but is included here to test the lexer's handling of long lines
