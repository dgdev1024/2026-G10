.global main

.org 0x80000000
    status: .dword 1

.org 0x2000
    mysub:
        ld d15, 0x0A11600D
        st [status], d15
        ret

    main:
        call mysub
        stop
