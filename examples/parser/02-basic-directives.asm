; Test 02: Basic Directives
; Tests the parser's ability to parse assembler directives.
; Directives control the assembler's behavior (origin address, data, exports).

; .org directive - sets the origin (location counter) address
.org 0x2000

; .global directive - exports labels for linking
.global main
.global helper_func

; .extern directive - imports labels from other modules
.extern external_func
.extern library_routine

; Multiple exports/imports on separate lines
.global export1
.global export2
.extern import1
.extern import2
; Define the exported labels
main:
    nop

helper_func:
    nop

export1:
    nop

export2:
    nop