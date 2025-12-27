; Test 08: Multi-File Basic - Main Module
; Tests: External symbol references, global symbol exports
;
; This is the main module that calls a function defined in another file.

.org 0x00002000

; Declare external function
.extern add_numbers

.global main

main:
    ; Set up parameters
    ld d0, 25               ; First parameter
    ld d1, 17               ; Second parameter
    
    ; Call external function
    call nc, add_numbers    ; Result in D0
    
    ; D0 should now be 42
    halt
