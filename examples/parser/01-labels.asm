; Test 01: Label Definitions
; Tests the parser's ability to parse label definitions.
; Labels are identifiers followed by a colon, marking specific locations
; in the code that can be referenced by instructions.

; Simple label
start:

; Another label
loop:

; Labels with underscores
my_label:
_private_label:
label_with_numbers_123:

; Multiple labels in sequence (each marks the same address)
first:
second:
third:

; Label followed by instruction on next line
entry_point:
    nop

; End of test
end_label:
