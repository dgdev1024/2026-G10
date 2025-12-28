.global main
.org 0x80000000
    counter: .byte 1
.org 0x2000
    target: .byte 0xAC
    loop:
        ld l0, [counter]
        inc l0
        st [counter], l0
        cmp l0, l1
        jpb zc, loop
        ret
    main:
        ld l1, [target]
        call loop
        stop
