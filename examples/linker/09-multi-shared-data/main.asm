; Test 09: Multi-File Shared Data - Main Module
; Tests: Accessing global data from another module
;
; This module accesses shared data defined in another file.

.org 0x00002000

; External data and functions
.extern shared_counter
.extern increment_counter
.extern get_counter

.global main

main:
    ; Get initial counter value
    call nc, get_counter    ; D0 = counter value
    
    ; Increment counter several times
    call nc, increment_counter
    call nc, increment_counter
    call nc, increment_counter
    
    ; Get final counter value
    call nc, get_counter    ; D0 should be 3 (assuming started at 0)
    
    halt
