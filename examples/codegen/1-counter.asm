.org 0x80000000
counter:
    .word 1           ; Reserve space for one uninitialized, 16-bit value.

.org 0x2000
start:
    ld w0, 0          ; Initialize W0 to 0
    st [counter], w0  ; Store 0 into counter

loop:
    ld w0, [counter]  ; Load counter value into W0
    inc w0            ; Increment counter
    st [counter], w0  ; Store updated counter back
    jpb zc, loop      ; Loop indefinitely until zero flag is set

done:
    jpb done          ; Infinite loop (same as `jpb nc, done`)
