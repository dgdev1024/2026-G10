; Test 09: Multi-File Shared Data - Counter Module
; Tests: Global data exports, global function exports
;
; This module provides a counter variable and functions to manipulate it.

; BSS section in RAM
.org 0x80000000

; Export the counter variable
.global shared_counter

shared_counter:
.dword 1                ; Reserve 1 dword (4 bytes)

; Code section
.org 0x00002100

.global increment_counter
.global get_counter

; Function: increment_counter
; Increments the shared counter by 1
increment_counter:
    ld d0, [shared_counter]
    inc d0
    st [shared_counter], d0
    ret

; Function: get_counter
; Returns the current counter value in D0
get_counter:
    ld d0, [shared_counter]
    ret
