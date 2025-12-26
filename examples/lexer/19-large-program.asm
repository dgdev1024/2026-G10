;
; 19-large-program.asm
; A larger program with multiple sections.
;

.org 0x1000
start:
    ld d0, 0
    call init
    call main_loop
    halt

init:
    ld d1, 10
    st [counter], d1
    ret

main_loop:
    ld d0, [counter]
    dec d0
    st [counter], d0
    cmp l0, 0
    jpb zc, main_loop
    ret

counter:
    .dword 0