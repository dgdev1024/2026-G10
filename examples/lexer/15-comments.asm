;
; 15-comments.asm
; Tests comments and whitespace handling.
;

; This is a comment
.org 0x1000  ; Another comment

label:  ; Comment after label
    ld d0, 0  ; Comment after instruction

; Multi-line comment
; Second line
    nop

; End comment