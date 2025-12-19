; Test 2: Operators - All operator types and multi-character sequences
; This test verifies correct tokenization of arithmetic, bitwise, logical,
; comparison, and assignment operators.

.ORG 0x3000

test_operators:
    ; Arithmetic operators
    ADD D0, D1          ; + (addition)
    SUB D2, D3          ; - (subtraction)
    ; MUL D4, D5        ; * (multiplication - if supported)
    ; DIV D6, D7        ; / (division - if supported)

    ; Bitwise operators
    AND D0, D1          ; & (bitwise AND)
    OR D2, D3           ; | (bitwise OR)
    XOR D4, D5          ; ^ (bitwise XOR)
    NOT D6              ; ~ (bitwise NOT)

    ; Shift operators (represented in operands)
    SLA D0              ; Shift left arithmetic
    SRA D1              ; Shift right arithmetic
    SRL D2              ; Shift right logical

    ; Comparison examples (for expressions - future parser use)
    ; a == b            ; == (equality)
    ; c != d            ; != (inequality)
    ; e < f             ; < (less than)
    ; g <= h            ; <= (less or equal)
    ; i > j             ; > (greater than)
    ; k >= l            ; >= (greater or equal)

    ; Assignment operators (for macro/expression use)
    ; x = 5             ; = (assignment)
    ; y += 3            ; += (add assign)
    ; z -= 2            ; -= (subtract assign)
    ; a *= 4            ; *= (multiply assign)
    ; b /= 2            ; /= (divide assign)
    ; c %= 3            ; %= (modulo assign)
    ; d &= 0xFF         ; &= (AND assign)
    ; e |= 0x10         ; |= (OR assign)
    ; f ^= 0x01         ; ^= (XOR assign)
    ; g <<= 2           ; <<= (left shift assign)
    ; h >>= 1           ; >>= (right shift assign)

    ; Logical operators (for conditional expressions)
    ; p && q            ; && (logical AND)
    ; r || s            ; || (logical OR)
    ; !t                ; ! (logical NOT)

    RET
