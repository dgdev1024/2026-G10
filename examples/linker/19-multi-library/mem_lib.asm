; Test 19: Library Pattern - Memory Library
; Tests: Memory utility functions
;
; Provides memory operations.
; Note: CMP uses L0, so size is passed in L0.

.org 0x00002300

.global mem_zero
.global mem_fill

; Function: mem_zero
; Input: D1 = address, L0 = size in bytes (max 255)
; Output: None (memory zeroed)
mem_zero:
    push d1
    push d2
    
    ld l2, 0                ; Zero value
    
mem_zero_loop:
    cmp l0, 0
    jpb zs, mem_zero_done   ; If size == 0, done
    
    st [d1], l2             ; Store zero byte
    inc d1                  ; Next address
    dec l0                  ; Decrement size
    jpb nc, mem_zero_loop

mem_zero_done:
    pop d2
    pop d1
    ret

; Function: mem_fill
; Input: D1 = address, L0 = size in bytes, L2 = fill value
; Output: None (memory filled)
mem_fill:
    push d1
    
mem_fill_loop:
    cmp l0, 0
    jpb zs, mem_fill_done   ; If size == 0, done
    
    st [d1], l2             ; Store fill byte
    inc d1                  ; Next address
    dec l0                  ; Decrement size
    jpb nc, mem_fill_loop

mem_fill_done:
    pop d1
    ret
