; Test 27: External Symbol References
; Tests that extern symbols generate proper relocation entries.

.extern external_function
.extern external_data
.extern library_init
.extern library_cleanup

.org 0x2000

; References to external functions
main:
    call nc, library_init       ; Call external function
    call nc, external_function  ; Another external call
    call nc, library_cleanup    ; Cleanup call
    ret nc

; Load external data address
load_external:
    ld d0, external_data        ; Load address of external symbol

; Jump to external
jump_external:
    jmp nc, external_function   ; Jump to external

; Mix of local and external references
mixed_references:
    call nc, local_helper       ; Local call
    call nc, external_function  ; External call
    call nc, another_local      ; Local call
    ret nc

local_helper:
    inc d0
    ret nc

another_local:
    dec d0
    ret nc

; Data section with local references only
; (External references in data require linker support)
.org 0x3000
function_table:
.dword local_helper             ; Local address
.dword another_local            ; Local address
.dword main                     ; Local address
