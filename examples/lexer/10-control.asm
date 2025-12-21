;
; 10-control.asm
; Tests control flow instructions.
;

.org 0x1000
control:
    nop
    stop
    halt
    di
    ei
    eii
    daa
    scf
    ccf
    clv
    sev