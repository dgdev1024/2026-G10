; Test 15: Circular Dependencies - Module B
; Tests: Circular references - Module B calls back to Module A
;
; Module B calls Module A, creating a circular call chain.
; Note: CMP uses L0, so depth is passed in L0.

; BSS section in RAM
.org 0x80000100

.global counter_b
counter_b:
.dword 1

; Code section
.org 0x00002200

; External reference to Module A
.extern function_a
.extern counter_a

.global function_b

; Function B: Increments counter_b and calls function_a if depth > 0
; Input: L0 = remaining depth
function_b:
    push d0                 ; Save D0 (contains L0)
    
    ; Increment counter_b
    ld d1, [counter_b]
    inc d1
    st [counter_b], d1
    
    ; Check if we should continue
    pop d0                  ; Restore L0
    cmp l0, 0
    jpb zs, function_b_done ; If depth == 0, done
    
    ; Call function_a with depth - 1
    dec l0
    call nc, function_a
    
function_b_done:
    ret
