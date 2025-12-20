; =============================================================================
; Test 7A: Multi-File Linking - Main Module
; =============================================================================
; This is the main module that references symbols from other files.
; Must be linked with: 7b-math-lib.asm
; 
; Demonstrates:
;   - .GLOBAL directive to export symbols
;   - .EXTERN directive to import symbols
;   - Cross-file symbol resolution
;   - Relocations across object files
; =============================================================================

; Declare external symbols (defined in 7b-math-lib.asm)
.extern add_numbers
.extern sub_numbers
.extern result_storage

; Export our entry point
.global main

; --- RAM Section ---
.org $80000000
operand1:
    .dword 1
operand2:
    .dword 1
add_result:
    .dword 1
sub_result:
    .dword 1

; --- ROM Section ---
.org $2000

main:
    ; Set up stack
    lsp $FFFFFFFC
    
    ; Initialize operands
    ld d0, 100
    st [operand1], d0
    
    ld d0, 37
    st [operand2], d0
    
    ; Call external add function
    ; D0 = first operand, D1 = second operand
    ld d0, [operand1]
    ld d1, [operand2]
    call add_numbers            ; External call!
    st [add_result], d0         ; Store result (should be 137)
    
    ; Call external subtract function
    ld d0, [operand1]
    ld d1, [operand2]
    call sub_numbers            ; External call!
    st [sub_result], d0         ; Store result (should be 63)
    
    ; Verify external data symbol
    ld d0, [result_storage]     ; Access external variable
    
    halt
