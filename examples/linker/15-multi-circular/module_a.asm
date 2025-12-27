; Test 15: Circular Dependencies - Module A
; Tests: Circular references between modules
;
; Module A calls Module B, which calls back to Module A.
; Note: CMP uses L0, so depth is passed in L0.

; BSS section in RAM
.org 0x80000000

.global counter_a
counter_a:
.dword 1

; Code section
.org 0x00002000

; External reference to Module B
.extern function_b
.extern counter_b

.global main
.global function_a

main:
    ; Initialize counters
    ld d0, 0
    st [counter_a], d0
    st [counter_b], d0
    
    ; Start the chain with function_a (use L0 for depth)
    ld l0, 5                ; Call depth
    call nc, function_a
    
    halt

; Function A: Increments counter_a and calls function_b if depth > 0
; Input: L0 = remaining depth
function_a:
    push d0                 ; Save D0 (contains L0)
    
    ; Increment counter_a
    ld d1, [counter_a]
    inc d1
    st [counter_a], d1
    
    ; Check if we should continue
    pop d0                  ; Restore L0
    cmp l0, 0
    jpb zs, function_a_done ; If depth == 0, done
    
    ; Call function_b with depth - 1
    dec l0
    call nc, function_b
    
function_a_done:
    ret
